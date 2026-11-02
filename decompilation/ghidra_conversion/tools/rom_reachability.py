#!/usr/bin/env python3
"""Conservative recursive-descent 68000 reachability for the US ROM.

Capstone decodes instructions directly from the ROM bytes. Direct branches and
calls, valid vector targets, and indirect call targets explicitly resolved by
the Ghidra call-site export are followed. Unresolved indirect transfers are
reported and are not guessed, so the resulting coverage is a lower bound.
"""

from __future__ import annotations

import argparse
import csv
from dataclasses import dataclass
import hashlib
from pathlib import Path
import re
import sys
from typing import Iterable

try:
    from capstone import CS_ARCH_M68K, CS_MODE_BIG_ENDIAN, CS_MODE_M68K_000, Cs
except ImportError as error:  # pragma: no cover - exercised in the CLI error path
    raise SystemExit(
        "rom_reachability.py requires the Python Capstone package with M68K support"
    ) from error


ROOT = Path(__file__).resolve().parents[3]
DEFAULT_ROM = ROOT / "decompilation/input/shadowrun_genesis_us.bin"
DEFAULT_FUNCTIONS = ROOT / "decompilation/export/functions.tsv"
DEFAULT_STATUS = ROOT / "decompilation/docs/ghidra/decompiled-function-status.tsv"
DEFAULT_CALL_SITES = ROOT / "decompilation/export/call-sites.tsv"
DEFAULT_DISASSEMBLY = ROOT / "decompilation/export/disassembly.s"
DEFAULT_REPORT = ROOT / "decompilation/docs/ghidra/rom-byte-reachability.md"
VECTOR_TABLE_END = 0x100

BRANCHES = {
    "bra", "bsr", "bcc", "bcs", "beq", "bne", "bge", "bgt", "bhi",
    "ble", "bls", "blt", "bmi", "bpl", "bvc", "bvs", "bhs", "blo",
    "dbcc", "dbcs", "dbeq", "dbne", "dbge", "dbgt", "dbhi", "dble",
    "dbls", "dblt", "dbmi", "dbpl", "dbvc", "dbvs", "dbra", "dbf",
    "dbt",
}
TERMINATORS = {"rts", "rte", "rtr", "stop", "illegal"}
HEX_OPERAND = re.compile(r"(?:\$|0x)([0-9a-fA-F]+)", re.IGNORECASE)


class ReachabilityError(Exception):
    pass


@dataclass(frozen=True)
class Instruction:
    address: int
    size: int
    mnemonic: str
    operands: str

    @property
    def end(self) -> int:
        return self.address + self.size

    @property
    def text(self) -> str:
        return f"{self.mnemonic} {self.operands}".strip()


@dataclass(frozen=True)
class VectorRoot:
    slot: int
    target: int


@dataclass(frozen=True)
class IndirectTransfer:
    address: int
    mnemonic: str
    operands: str
    target: int | None = None
    status: str = "unresolved"
    callee: str = ""


@dataclass(frozen=True)
class IndirectJumpSite:
    address: int
    operands: str
    reached: bool
    table_base: int | None = None
    targets: tuple[int, ...] = ()
    bounded: bool = True


@dataclass(frozen=True)
class ListingInstruction:
    address: int
    mnemonic: str
    operands: str


@dataclass(frozen=True)
class DispatchTable:
    site: int
    kind: str
    base: int
    targets: tuple[int, ...]
    evidence: tuple[int, ...] = ()
    bounded: bool = True


@dataclass(frozen=True)
class FunctionEntry:
    address: int
    size: int | None
    name: str


@dataclass
class Analysis:
    rom_size: int
    rom_sha256: str
    vector_roots: list[VectorRoot]
    instructions: dict[int, Instruction]
    resolved_indirect: list[IndirectTransfer]
    indirect_jumps: list[IndirectJumpSite]
    dispatch_tables: list[DispatchTable]
    unresolved_calls: list[IndirectTransfer]
    bad_targets: set[int]
    decode_failures: set[int]
    function_entries: list[FunctionEntry]
    status_entries: set[int]
    function_ranges: list[tuple[int, int]]
    outside_ranges: list[tuple[int, int]]

    @property
    def reached_bytes(self) -> int:
        return sum(end - start for start, end in merge_intervals(
            [(ins.address, ins.end) for ins in self.instructions.values()]
        ))

    @property
    def outside_bytes(self) -> int:
        return sum(end - start for start, end in self.outside_ranges)

    @property
    def known_function_bytes_total(self) -> int:
        return sum(entry.size or 0 for entry in self.function_entries)

    @property
    def known_function_bytes_reached(self) -> int:
        reached = merge_intervals((ins.address, ins.end) for ins in self.instructions.values())
        return sum(
            end - start
            for entry in self.function_entries
            if entry.size
            for start, end in intersect_intervals(
                reached, [(entry.address, entry.address + entry.size)]
            )
        )

    @property
    def unreached_functions(self) -> list[FunctionEntry]:
        reached = merge_intervals((ins.address, ins.end) for ins in self.instructions.values())
        return [
            entry
            for entry in self.function_entries
            if entry.size
            and not intersect_intervals(
                reached, [(entry.address, entry.address + entry.size)]
            )
        ]

    @property
    def unresolved_jumps(self) -> list[IndirectJumpSite]:
        return [site for site in self.indirect_jumps if not site.targets or not site.bounded]


def normalize_address(value: str, source: str) -> int:
    text = value.strip()
    if text.lower().startswith("0x"):
        text = text[2:]
    if not text or not re.fullmatch(r"[0-9a-fA-F]+", text):
        raise ReachabilityError(f"{source}: invalid address {value!r}")
    return int(text, 16)


def read_tsv(path: Path, required: set[str]) -> list[dict[str, str]]:
    if not path.is_file():
        raise ReachabilityError(f"missing input TSV: {path}")
    with path.open(newline="", encoding="utf-8") as stream:
        reader = csv.DictReader(stream, delimiter="\t")
        if reader.fieldnames is None or not required.issubset(reader.fieldnames):
            expected = ", ".join(sorted(required))
            raise ReachabilityError(f"{path}: expected TSV columns {expected}")
        return list(reader)


def read_function_inventory(functions_path: Path, status_path: Path) -> tuple[list[FunctionEntry], set[int]]:
    functions: dict[int, FunctionEntry] = {}
    for number, row in enumerate(read_tsv(functions_path, {"entry", "name", "size"}), start=2):
        address = normalize_address(row["entry"], f"{functions_path}:{number}")
        try:
            size = int(row["size"], 0) if row["size"].strip().lower().startswith("0x") else int(row["size"])
        except ValueError as error:
            raise ReachabilityError(f"{functions_path}:{number}: invalid size {row['size']!r}") from error
        if address in functions:
            raise ReachabilityError(f"{functions_path}:{number}: duplicate function entry 0x{address:06X}")
        if size < 0:
            raise ReachabilityError(f"{functions_path}:{number}: negative function size")
        functions[address] = FunctionEntry(address, size, row["name"].strip())

    status_rows = read_tsv(status_path, {"rom_entry"})
    status_entries = {
        normalize_address(row["rom_entry"], f"{status_path}:{number}")
        for number, row in enumerate(status_rows, start=2)
    }
    return sorted(functions.values(), key=lambda entry: entry.address), status_entries


def read_indirect_call_targets(path: Path) -> tuple[dict[int, list[tuple[int, str]]], dict[int, dict[str, str]]]:
    rows = read_tsv(path, {"instruction", "target", "callee", "status"})
    targets: dict[int, list[tuple[int, str]]] = {}
    metadata: dict[int, dict[str, str]] = {}
    for number, row in enumerate(rows, start=2):
        instruction = normalize_address(row["instruction"], f"{path}:{number}")
        metadata[instruction] = row
        if row["status"].strip().lower() != "resolved" or not row["target"].strip():
            continue
        target = normalize_address(row["target"], f"{path}:{number}")
        targets.setdefault(instruction, []).append((target, row["callee"].strip()))
    for address in targets:
        targets[address] = sorted(set(targets[address]))
    return targets, metadata


def read_listing(path: Path) -> list[ListingInstruction]:
    listing: list[ListingInstruction] = []
    line_pattern = re.compile(r"^\s*([0-9a-fA-F]{1,8}):\s*([A-Za-z][A-Za-z0-9.]*)\s*(.*?)\s*$")
    if not path.is_file():
        raise ReachabilityError(f"missing Ghidra disassembly listing: {path}")
    with path.open(encoding="utf-8", errors="replace") as stream:
        for line in stream:
            match = line_pattern.match(line)
            if not match:
                continue
            operands = match.group(3).split(";", 1)[0].strip()
            listing.append(ListingInstruction(int(match.group(1), 16), match.group(2).lower(), operands))
    return sorted(listing, key=lambda item: item.address)


def indirect_register(operands: str) -> str | None:
    match = re.fullmatch(r"\(\s*(a[0-7])\s*\)", operands, re.IGNORECASE)
    return match.group(1).lower() if match else None


def absolute_lea(operands: str) -> tuple[int, str] | None:
    match = re.fullmatch(r"\(\s*0x([0-9a-fA-F]+)\s*\)\.l\s*,\s*(a[0-7])", operands, re.IGNORECASE)
    if not match:
        return None
    return int(match.group(1), 16), match.group(2).lower()


def pointer_load_details(operands: str) -> tuple[str, str, int, str | None, int] | None:
    match = re.fullmatch(
        r"\(\s*(?:(0x[0-9a-fA-F]+|\$[0-9a-fA-F]+)\s*,\s*)?"
        r"(a[0-7])(?:\s*,\s*(d[0-7])\.?([wl])(?:\s*\*\s*(0x[0-9a-fA-F]+|\$[0-9a-fA-F]+))?)?\s*\)\s*,\s*(a[0-7])",
        operands,
        re.IGNORECASE,
    )
    if not match or match.group(2).lower() != match.group(6).lower():
        return None
    displacement = int(match.group(1).replace("$", "0x"), 16) if match.group(1) else 0
    encoded_scale = int(match.group(5).replace("$", "0x"), 16) if match.group(5) else 1
    return (
        match.group(2).lower(), match.group(6).lower(), displacement,
        match.group(3).lower() if match.group(3) else None, encoded_scale,
    )


def pointer_load_register(operands: str) -> str | None:
    details = pointer_load_details(operands)
    return details[0] if details else None


def pointer_table_stride(
    listing: list[ListingInstruction], load_index: int, details: tuple[str, str, int, str | None, int]
) -> int:
    index_register = details[3]
    encoded_scale = details[4]
    if index_register is None:
        return 4
    shift_pattern = re.compile(r"#(?:0x|\$)?([0-9a-fA-F]+)\s*,\s*(d[0-7])w", re.IGNORECASE)
    for item in reversed(listing[max(0, load_index - 24):load_index]):
        if item.mnemonic.split(".", 1)[0] not in {"lsl", "asl"}:
            continue
        match = shift_pattern.fullmatch(item.operands)
        if match and match.group(2).lower() == index_register:
            amount = int(match.group(1), 16)
            if amount < 8:
                return encoded_scale * (1 << amount)
            break
    return encoded_scale


def read_pointer_table_targets(
    rom: bytes,
    base: int,
    stride: int,
    field_offset: int,
    boundaries: list[int],
    count_limit: int | None = None,
) -> tuple[int, ...]:
    """Read pointer fields at a fixed record stride, preserving null slots."""
    following = next((boundary for boundary in boundaries if boundary > base), None)
    upper = following if following is not None else min(len(rom), base + stride * 64)
    if count_limit is not None:
        count = count_limit
    elif following is not None:
        available = upper - base - field_offset
        count = max(0, (available - 4) // stride + 1) if available >= 4 else 0
    else:
        count = 64

    disassembler = Cs(CS_ARCH_M68K, CS_MODE_M68K_000 | CS_MODE_BIG_ENDIAN)
    targets: list[int] = []
    for index in range(count):
        entry = base + field_offset + stride * index
        if entry + 4 > min(upper, len(rom)):
            break
        target = int.from_bytes(rom[entry:entry + 4], "big")
        if target == 0 and (count_limit is not None or following is not None):
            continue
        if target < VECTOR_TABLE_END or target >= len(rom) or target & 1:
            break
        decoded = decode_one(disassembler, rom, target)
        if decoded is None or decoded.mnemonic.lower() == "dc.w":
            break
        targets.append(target)
    return tuple(targets)


def guarded_table_count(listing: list[ListingInstruction], site: int, table_base: int) -> int | None:
    """Infer an indexed table's count only from a nearby explicit ROM guard.

    The recognized shapes are a compare against the maximum valid index with
    a greater-than branch around the dispatch, or a less-than branch directly
    into the dispatch block. Other condition codes and control-flow shapes are
    left unbounded.
    """
    site_index = next((index for index, item in enumerate(listing) if item.address == site), None)
    if site_index is None:
        return None
    table_index = None
    for index, item in enumerate(listing):
        base = absolute_lea(item.operands) if item.mnemonic in {"lea", "lea.l"} else None
        if item.address <= site and base and base[0] == table_base:
            table_index = index
    if table_index is None:
        return None

    lower = max(0, table_index - 40)
    for index in range(table_index - 1, lower - 1, -1):
        compare = listing[index]
        match = re.fullmatch(r"#(?:0x|\$)?([0-9a-fA-F]+)\s*,\s*(.+)", compare.operands, re.IGNORECASE)
        if compare.mnemonic not in {"cmpi", "cmpi.b", "cmpi.w", "cmpi.l"} or not match:
            continue
        immediate = int(match.group(1), 16)
        branch_index = index + 1
        if branch_index >= len(listing):
            continue
        branch = listing[branch_index]
        branch_mnemonic = branch.mnemonic.split(".", 1)[0]
        if not branch_mnemonic.startswith("b") or branch_mnemonic in {"bra", "bsr", "btst"}:
            continue
        target = parse_direct_target(branch.operands)
        if target is None:
            continue
        # A less-than branch into the guarded table block bounds the index to
        # [0, immediate). This covers the ROM's signed byte menu dispatch.
        if branch_mnemonic in {"bmi", "blo", "bcs"} and target == listing[table_index].address:
            return immediate
        # A greater-than branch around the dispatch bounds a zero-based
        # inclusive index to [0, immediate]. The ROM's command dispatcher
        # uses BPL after forming a nonnegative index and rejects index 0xFA,
        # so that exclusive ceiling is the count.
        if branch_mnemonic == "bpl" and target > site:
            return immediate
        if branch_mnemonic in {"bgt", "bhi"} and target > site:
            return immediate + 1
    return None


def direct_call_target(item: ListingInstruction) -> int | None:
    if item.mnemonic.split(".", 1)[0] not in {"jsr", "bsr", "bra"}:
        return None
    return parse_direct_target(item.operands)


def caller_supplied_table_bases(
    listing: list[ListingInstruction], function_starts: set[int],
    transfer_sites: dict[int, tuple[str, str, int, int]],
) -> dict[int, list[tuple[str, int, int, int, int]]]:
    """Find tables passed in A0 to small shared indirect-call dispatchers.

    For each indirect transfer, locate its containing known function entry.
    Direct callers that set the dispatcher's table register with an absolute
    LEA immediately before the call provide additional table bases.
    """
    result: dict[int, list[tuple[str, int, int, int, int]]] = {}
    for transfer_site, (kind, register, stride, field_offset) in transfer_sites.items():
        function_start = max(
            (start for start in function_starts if start <= transfer_site),
            default=None,
        )
        if function_start is None:
            continue
        for call_index, call in enumerate(listing):
            if direct_call_target(call) != function_start or call_index == 0:
                continue
            setup = listing[call_index - 1]
            table = absolute_lea(setup.operands) if setup.mnemonic in {"lea", "lea.l"} else None
            if not table or table[1] != register:
                continue
            result.setdefault(transfer_site, []).append(
                (kind, table[0], setup.address, stride, field_offset)
            )
    for site in result:
        result[site] = sorted(set(result[site]))
    return result


def discover_schedule_tables(
    rom: bytes, listing: list[ListingInstruction]
) -> dict[int, list[DispatchTable]]:
    """Resolve title-schedule callbacks stored as delay/longword records."""
    schedule_stores: list[tuple[int, int]] = []
    store_pattern = re.compile(
        r"#(?:0x|\$)([0-9a-fA-F]+)\s*,\s*\(0x0*ffdd32\)\.l",
        re.IGNORECASE,
    )
    for item in listing:
        if item.mnemonic != "move.l":
            continue
        match = store_pattern.fullmatch(item.operands)
        if match:
            schedule_stores.append((int(match.group(1), 16), item.address))

    callback_rows: dict[int, tuple[int, ...]] = {}
    disassembler = Cs(CS_ARCH_M68K, CS_MODE_M68K_000 | CS_MODE_BIG_ENDIAN)
    for base, _ in schedule_stores:
        targets: list[int] = []
        offset = base
        for _ in range(256):
            if offset + 6 > len(rom):
                break
            delay = int.from_bytes(rom[offset:offset + 2], "big")
            if delay == 0:
                break
            target = int.from_bytes(rom[offset + 2:offset + 6], "big")
            if target >= len(rom) or target & 1 or decode_one(disassembler, rom, target) is None:
                break
            targets.append(target)
            offset += 6
        if targets:
            callback_rows[base] = tuple(targets)

    site = 0x5AF48
    if not callback_rows or not any(item.address == site for item in listing):
        return {}
    result: dict[int, list[DispatchTable]] = {site: []}
    for base, targets in sorted(callback_rows.items()):
        store_addresses = tuple(address for value, address in schedule_stores if value == base)
        result[site].append(
            DispatchTable(
                site, "jsr schedule delay/callback records", base, targets,
                evidence=(base, *store_addresses),
            )
        )
    return result


def discover_effect_slot_callbacks(listing: list[ListingInstruction]) -> dict[int, list[DispatchTable]]:
    """Resolve callbacks written into the active effect slots.

    The service at 0x163DA walks ten records rooted at RAM 0xFFD740. Its
    callback field is +8; the ROM installs the timer and palette handlers at
    0x164C8 and 0x16662.
    """
    site = 0x1640A
    by_address = {item.address: item for item in listing}
    if site not in by_address:
        return {}
    writer_addresses = (0x164C8, 0x16662)
    targets: list[int] = []
    for address in writer_addresses:
        item = by_address.get(address)
        if not item or item.mnemonic != "move.l":
            continue
        match = re.fullmatch(
            r"#(?:0x|\$)([0-9a-fA-F]+)\s*,\s*\(0x0*8,A0\)",
            item.operands,
            re.IGNORECASE,
        )
        if match:
            targets.append(int(match.group(1), 16))
    if not targets:
        return {}
    return {
        site: [DispatchTable(
            site, "jsr active-record callback (+0x8)", 0xFFD740,
            tuple(targets), evidence=writer_addresses,
        )]
    }


def raw_absolute_lea(operands: str) -> tuple[int, str] | None:
    match = re.fullmatch(
        r"(?:\$|0x)([0-9a-fA-F]+)\.l\s*,\s*(a[0-7])",
        operands,
        re.IGNORECASE,
    )
    return (int(match.group(1), 16), match.group(2).lower()) if match else None


def discover_runtime_pointer_table(
    rom: bytes,
    instructions: dict[int, Instruction],
    site: int,
    listing: list[ListingInstruction],
) -> DispatchTable | None:
    """Resolve an indexed pointer field in code present in raw ROM but absent from Ghidra export."""
    call = instructions.get(site)
    register = indirect_register(call.operands) if call else None
    if not call or call.mnemonic.split(".", 1)[0] != "jsr" or not register:
        return None
    load = next((item for item in instructions.values() if item.end == site), None)
    if not load or load.mnemonic.split(".", 1)[0] != "movea":
        return None
    details = pointer_load_details(load.operands)
    if not details or details[0] != register:
        return None
    base_register, _, displacement, index_register, encoded_scale = details
    if not index_register:
        return None

    prior = sorted(
        (item for item in instructions.values() if item.address < load.address and load.address - item.address < 0x50),
        key=lambda item: item.address,
    )
    lea_index = next((
        index for index in range(len(prior) - 1, -1, -1)
        if prior[index].mnemonic.split(".", 1)[0] == "lea"
        and (source := raw_absolute_lea(prior[index].operands)) is not None
        and source[1] == base_register
    ), None)
    if lea_index is None:
        return None
    table_base, _ = raw_absolute_lea(prior[lea_index].operands)  # type: ignore[misc]
    for item in prior[lea_index + 1:]:
        mnemonic = item.mnemonic.split(".", 1)[0]
        if mnemonic in {"movea", "lea", "adda", "addq", "addi", "suba", "subq", "subi"}:
            destination = item.operands.rsplit(",", 1)[-1].strip().lower()
            if destination == base_register:
                return None

    shift_pattern = re.compile(
        r"#(?:0x|\$)?([0-9a-fA-F]+)\s*,\s*(d[0-7])(?:[wl])?",
        re.IGNORECASE,
    )
    shift_index = None
    shift_amount = 0
    for index in range(lea_index + 1, len(prior)):
        item = prior[index]
        if item.mnemonic.split(".", 1)[0] not in {"lsl", "asl"}:
            continue
        match = shift_pattern.fullmatch(item.operands)
        if match and match.group(2).lower() == index_register:
            shift_index = index
            shift_amount = int(match.group(1), 16)
    if shift_amount >= 8:
        return None
    stride = encoded_scale * (1 << shift_amount)
    field_offset = displacement
    add_pattern = re.compile(
        r"#(?:0x|\$)?([0-9a-fA-F]+)\s*,\s*(d[0-7])(?:[wl])?",
        re.IGNORECASE,
    )
    if shift_index is not None:
        for item in prior[shift_index + 1:]:
            if item.mnemonic.split(".", 1)[0] not in {"addq", "addi"}:
                continue
            match = add_pattern.fullmatch(item.operands)
            if match and match.group(2).lower() == index_register:
                field_offset += int(match.group(1), 16)

    boundaries = sorted({
        base[0]
        for item in listing
        if item.mnemonic in {"lea", "lea.l"}
        and (base := absolute_lea(item.operands)) is not None
    } | {table_base})
    targets = read_pointer_table_targets(rom, table_base, stride, field_offset, boundaries)
    if not targets:
        return None
    return DispatchTable(
        site, f"jsr runtime record-field dispatch (stride {stride}, offset {field_offset})",
        table_base, targets, evidence=(table_base, prior[lea_index].address, load.address),
    )


def pc_relative_lea_target(item: ListingInstruction) -> tuple[int, str] | None:
    match = re.fullmatch(
        r"\(\s*(-?(?:0x|\$)?[0-9a-fA-F]+)\s*,\s*pc\s*\)\s*,\s*(a[0-7])",
        item.operands,
        re.IGNORECASE,
    )
    if not match:
        return None
    displacement_text = match.group(1)
    sign = -1 if displacement_text.startswith("-") else 1
    displacement_text = displacement_text.lstrip("-")
    if displacement_text.lower().startswith("0x"):
        displacement_text = displacement_text[2:]
    elif displacement_text.startswith("$"):
        displacement_text = displacement_text[1:]
    displacement = sign * int(displacement_text, 16)
    # 68000 PC-relative effective addresses use the extension-word address.
    return item.address + 2 + displacement, match.group(2).lower()


def discover_pc_relative_word_dispatches(
    rom: bytes, listing: list[ListingInstruction], function_starts: set[int]
) -> dict[int, list[DispatchTable]]:
    """Resolve PC-relative signed-word target offsets used by the 0x24E20 helper."""
    result: dict[int, list[DispatchTable]] = {}
    disassembler = Cs(CS_ARCH_M68K, CS_MODE_M68K_000 | CS_MODE_BIG_ENDIAN)
    for site_index, call in enumerate(listing):
        call_register = indirect_register(call.operands) if call.mnemonic == "jsr" else None
        if not call_register:
            continue
        table_use = None
        use_index = None
        for index in range(site_index - 1, max(-1, site_index - 24), -1):
            item = listing[index]
            if item.mnemonic not in {"adda.w", "add.w"}:
                continue
            match = re.fullmatch(
                r"\(\s*(0x[0-9a-fA-F]+|\$[0-9a-fA-F]+)\s*,\s*pc\s*,\s*(d[0-7])w(?:\s*\*\s*(?:0x|\$)?[0-9a-fA-F]+)?\s*\)\s*,\s*(a[0-7])",
                item.operands,
                re.IGNORECASE,
            )
            if match and match.group(3).lower() == call_register:
                table_use = (int(match.group(1).replace("$", "0x"), 16), match.group(3).lower())
                use_index = index
                break
        if table_use is None or use_index is None:
            continue
        table_base = table_use[0]
        lower_bound = max((start for start in function_starts if start <= call.address), default=0)
        code_base = None
        lea_address = None
        for item in reversed(listing[max(0, use_index - 24):use_index + 1]):
            if item.address < lower_bound or item.mnemonic not in {"lea", "lea.l"}:
                continue
            target = pc_relative_lea_target(item)
            if target and target[1] == call_register:
                code_base, lea_address = target[0], item.address
                break
        if code_base is None or lea_address is None or table_base + 2 > len(rom):
            continue
        first_offset = int.from_bytes(rom[table_base:table_base + 2], "big", signed=True)
        table_end = code_base + first_offset
        if table_end <= table_base or (table_end - table_base) % 2:
            continue
        count = (table_end - table_base) // 2
        if count <= 0 or count > 64 or table_base + 2 * count > len(rom):
            continue
        targets: list[int] = []
        for index in range(count):
            offset = int.from_bytes(rom[table_base + index * 2:table_base + index * 2 + 2], "big", signed=True)
            target = code_base + offset
            decoded = decode_one(disassembler, rom, target)
            if target < table_end or decoded is None or decoded.mnemonic.lower() == "dc.w":
                targets = []
                break
            targets.append(target)
        if targets:
            result.setdefault(call.address, []).append(
                DispatchTable(
                    call.address, "jsr PC-relative word-offset table", table_base,
                    tuple(targets), evidence=(call.address, lea_address, table_base),
                )
            )
    return result


def pc_indexed_jump_candidate(
    rom: bytes, listing: list[ListingInstruction], site_index: int
) -> DispatchTable | None:
    """Recognize the 18-byte inline dispatch idiom used at ROM 0x2510A.

    Its code establishes a fixed block stride, but the source record length
    has no local upper-bound check, so these starts remain candidates rather
    than proven targets.
    """
    item = listing[site_index]
    match = re.fullmatch(
        r"\(\s*(0x[0-9a-fA-F]+|\$[0-9a-fA-F]+)\s*,\s*pc\s*,\s*d([0-7])[wl](?:\s*\*\s*(?:0x|\$)?[0-9a-fA-F]+)?\s*\)",
        item.operands,
        re.IGNORECASE,
    )
    if item.mnemonic != "jmp" or not match:
        return None
    base = int(match.group(1).replace("$", "0x"), 16)
    index_register = f"d{match.group(2)}"
    if site_index < 4:
        return None
    previous = listing[site_index - 4:site_index]
    double = re.fullmatch(r"(d[0-7])w\s*,\s*\1w", previous[0].operands, re.IGNORECASE)
    saved = re.fullmatch(r"(d[0-7])w\s*,\s*(d[0-7])w", previous[1].operands, re.IGNORECASE)
    shift = re.fullmatch(r"#(?:0x|\$)?([0-9a-fA-F]+)\s*,\s*(d[0-7])w", previous[2].operands, re.IGNORECASE)
    add_saved = re.fullmatch(r"(d[0-7])w\s*,\s*(d[0-7])w", previous[3].operands, re.IGNORECASE)
    if (
        previous[0].mnemonic != "add.w"
        or previous[1].mnemonic != "move.w"
        or previous[2].mnemonic != "lsl.w"
        or previous[3].mnemonic != "add.w"
        or not double or not saved or not shift or not add_saved
    ):
        return None
    if (
        double.group(1).lower() != index_register
        or saved.group(1).lower() != index_register
        or shift.group(2).lower() != index_register
        or add_saved.group(2).lower() != index_register
        or saved.group(2).lower() != add_saved.group(1).lower()
    ):
        return None
    shift_amount = int(shift.group(1), 16)
    stride = 2 * ((1 << shift_amount) + 1)
    if base != item.address + 4 or stride <= 0:
        return None

    # The preceding zero-value branch exits after the inline block region.
    end = None
    evidence = [item.address]
    for branch in reversed(listing[max(0, site_index - 24):site_index]):
        mnemonic = branch.mnemonic.split(".", 1)[0]
        if not mnemonic.startswith("b") or mnemonic in {"bra", "bsr", "btst"}:
            continue
        target = parse_direct_target(branch.operands)
        if target is not None and target > base and (target - base) % stride == 0:
            end = target
            evidence.append(branch.address)
            break
    if end is None:
        return None
    count = (end - base) // stride
    if count <= 0:
        return None
    disassembler = Cs(CS_ARCH_M68K, CS_MODE_M68K_000 | CS_MODE_BIG_ENDIAN)
    targets = tuple(base + stride * index for index in range(count))
    if any(decode_one(disassembler, rom, target) is None for target in targets):
        return None
    return DispatchTable(
        item.address, "jmp inline fixed-stride candidates", base, targets,
        evidence=tuple(sorted(evidence)), bounded=False,
    )


def discover_dispatch_tables(
    rom: bytes,
    listing: list[ListingInstruction],
    function_starts: set[int],
) -> dict[int, list[DispatchTable]]:
    """Resolve source pointer tables and known indirect callback records.

    Pointer-table loads can be direct or indexed, either locally based with
    `lea table,An` or passed by a caller to a small dispatcher. The title
    scheduler and active effect slots use compact record/callback fields and
    are handled as their source-specific data structures.
    """
    indexes = {item.address: index for index, item in enumerate(listing)}
    transfer_bases: dict[int, list[tuple[str, int, int, int, int]]] = {}
    table_bases: set[int] = set()
    transfer_sites: dict[int, tuple[str, str, int, int]] = {}
    for item in listing:
        if item.mnemonic not in {"jmp", "jsr"}:
            continue
        register = indirect_register(item.operands)
        if not register:
            continue
        transfer_sites[item.address] = (item.mnemonic, register, 4, 0)
        index = indexes[item.address]
        lower_bound = max((start for start in function_starts if start <= item.address), default=0)
        first = max(0, index - 32)
        for load_index in range(index - 1, first - 1, -1):
            load = listing[load_index]
            if load.address < lower_bound:
                break
            details = pointer_load_details(load.operands) if load.mnemonic == "movea.l" else None
            if not details or details[0] != register:
                continue
            stride = pointer_table_stride(listing, load_index, details)
            field_offset = details[2]
            transfer_sites[item.address] = (item.mnemonic, register, stride, field_offset)
            for lea_index in range(load_index - 1, first - 1, -1):
                lea = listing[lea_index]
                if lea.address < lower_bound:
                    break
                if lea.mnemonic not in {"lea", "lea.l"}:
                    continue
                base = absolute_lea(lea.operands)
                if base and base[1] == register:
                    transfer_bases.setdefault(item.address, []).append(
                        (item.mnemonic, base[0], lea.address, stride, field_offset)
                    )
                    table_bases.add(base[0])
                    break
            if item.address in transfer_bases:
                break

    for site, bases in caller_supplied_table_bases(listing, function_starts, transfer_sites).items():
        for kind, base, evidence, stride, field_offset in bases:
            transfer_bases.setdefault(site, []).append(
                (kind, base, evidence, stride, field_offset)
            )
            table_bases.add(base)

    referenced_bases = {
        base[0]
        for item in listing
        if item.mnemonic in {"lea", "lea.l"}
        and (base := absolute_lea(item.operands)) is not None
    }
    ordered_boundaries = sorted(table_bases | referenced_bases)
    targets_by_geometry: dict[tuple[int, int, int], tuple[int, ...]] = {}
    counts_by_geometry: dict[tuple[int, int, int], int] = {}
    for site, uses in transfer_bases.items():
        for _, base, _, stride, field_offset in uses:
            count = guarded_table_count(listing, site, base)
            geometry = (base, stride, field_offset)
            if count is not None:
                counts_by_geometry[geometry] = max(counts_by_geometry.get(geometry, 0), count)
    geometries = sorted({(base, stride, field_offset) for uses in transfer_bases.values()
                         for _, base, _, stride, field_offset in uses})
    for base, stride, field_offset in geometries:
        count_limit = counts_by_geometry.get((base, stride, field_offset))
        targets_by_geometry[(base, stride, field_offset)] = read_pointer_table_targets(
            rom, base, stride, field_offset, ordered_boundaries, count_limit
        )

    result: dict[int, list[DispatchTable]] = {}
    for site, uses in transfer_bases.items():
        for kind, base, source_address, stride, field_offset in sorted(set(uses)):
            targets = targets_by_geometry.get((base, stride, field_offset), ())
            if targets:
                result.setdefault(site, []).append(
                    DispatchTable(site, f"{kind} longword-pointer table (stride {stride})", base, targets,
                                  evidence=(base, source_address))
                )

    for extra in (discover_effect_slot_callbacks(listing), discover_schedule_tables(rom, listing)):
        for site, tables in extra.items():
            result.setdefault(site, []).extend(tables)

    for site, tables in discover_pc_relative_word_dispatches(rom, listing, function_starts).items():
        result.setdefault(site, []).extend(tables)

    pc_tables = [
        candidate
        for index, item in enumerate(listing)
        if (candidate := pc_indexed_jump_candidate(rom, listing, index)) is not None
    ]
    for table in pc_tables:
        result.setdefault(table.site, []).append(table)

    for site in result:
        unique = {(table.kind, table.base, table.targets): table for table in result[site]}
        result[site] = sorted(unique.values(), key=lambda table: (table.base, table.kind))
    return result


def vector_roots(rom: bytes) -> list[VectorRoot]:
    if len(rom) < VECTOR_TABLE_END:
        raise ReachabilityError("ROM is too short to contain the 68000 vector table")
    roots: list[VectorRoot] = []
    # Slot zero is the initial stack pointer, not an instruction address.
    for slot in range(4, VECTOR_TABLE_END, 4):
        target = int.from_bytes(rom[slot:slot + 4], "big")
        if target == 0 or target >= len(rom) or target & 1:
            continue
        roots.append(VectorRoot(slot, target))
    return roots


def merge_intervals(intervals: Iterable[tuple[int, int]]) -> list[tuple[int, int]]:
    ordered = sorted((start, end) for start, end in intervals if end > start)
    if not ordered:
        return []
    merged: list[tuple[int, int]] = []
    start, end = ordered[0]
    for next_start, next_end in ordered[1:]:
        if next_start <= end:
            end = max(end, next_end)
        else:
            merged.append((start, end))
            start, end = next_start, next_end
    merged.append((start, end))
    return merged


def subtract_intervals(
    source: Iterable[tuple[int, int]], masks: Iterable[tuple[int, int]]
) -> list[tuple[int, int]]:
    result: list[tuple[int, int]] = []
    merged_masks = merge_intervals(masks)
    for start, end in merge_intervals(source):
        cursor = start
        for mask_start, mask_end in merged_masks:
            if mask_end <= cursor:
                continue
            if mask_start >= end:
                break
            if mask_start > cursor:
                result.append((cursor, min(mask_start, end)))
            cursor = max(cursor, mask_end)
            if cursor >= end:
                break
        if cursor < end:
            result.append((cursor, end))
    return result


def parse_direct_target(operands: str) -> int | None:
    if "(" in operands:
        return None
    match = HEX_OPERAND.search(operands)
    return int(match.group(1), 16) if match else None


def is_indirect(operands: str) -> bool:
    return "(" in operands


def decode_one(disassembler: Cs, rom: bytes, address: int):
    if address < 0 or address + 2 > len(rom) or address & 1:
        return None
    window = rom[address:min(len(rom), address + 16)]
    return next(disassembler.disasm(window, address, count=1), None)


def analyze_rom(
    rom: bytes,
    functions_path: Path,
    status_path: Path,
    call_sites_path: Path,
    disassembly_path: Path,
) -> Analysis:
    roots = vector_roots(rom)
    functions, status_entries = read_function_inventory(functions_path, status_path)
    indirect_targets, call_site_metadata = read_indirect_call_targets(call_sites_path)
    listing = read_listing(disassembly_path)
    function_starts = {entry.address for entry in functions} | status_entries
    dispatch_by_site = discover_dispatch_tables(rom, listing, function_starts)
    function_names = {entry.address: entry.name for entry in functions}
    disassembler = Cs(CS_ARCH_M68K, CS_MODE_M68K_000 | CS_MODE_BIG_ENDIAN)
    disassembler.detail = True

    pending = [root.target for root in roots]
    enqueued = set(pending)
    instructions: dict[int, Instruction] = {}
    resolved_indirect: dict[tuple[int, int], IndirectTransfer] = {}
    unresolved_calls: dict[int, IndirectTransfer] = {}
    bad_targets: set[int] = set()
    decode_failures: set[int] = set()

    def add_target(target: int) -> None:
        if target < 0 or target >= len(rom) or target & 1:
            bad_targets.add(target)
            return
        if target not in enqueued:
            enqueued.add(target)
            pending.append(target)

    while pending:
        address = pending.pop()
        if address in instructions:
            continue
        decoded = decode_one(disassembler, rom, address)
        if decoded is None or decoded.size <= 0 or decoded.mnemonic.lower() == "dc.w":
            decode_failures.add(address)
            continue
        instruction = Instruction(address, decoded.size, decoded.mnemonic.lower(), decoded.op_str)
        instructions[address] = instruction
        next_address = instruction.end
        mnemonic = instruction.mnemonic.split(".", 1)[0]
        operands = instruction.operands

        if mnemonic in TERMINATORS:
            continue

        if mnemonic == "jmp":
            target = parse_direct_target(operands)
            if target is not None:
                add_target(target)
            else:
                for dispatch in dispatch_by_site.get(address, []):
                    if dispatch.bounded:
                        for resolved_target in dispatch.targets:
                            add_target(resolved_target)
            continue

        if mnemonic == "jsr":
            target = parse_direct_target(operands)
            if target is not None:
                add_target(target)
            elif is_indirect(operands):
                resolved = indirect_targets.get(address, [])
                if resolved:
                    for resolved_target, callee in resolved:
                        add_target(resolved_target)
                        resolved_indirect[(address, resolved_target)] = IndirectTransfer(
                            address, mnemonic, operands, resolved_target, "resolved", callee
                        )
                else:
                    dispatches = dispatch_by_site.get(address, [])
                    if not any(dispatch.bounded for dispatch in dispatches):
                        runtime_dispatch = discover_runtime_pointer_table(
                            rom, instructions, address, listing
                        )
                        if runtime_dispatch:
                            dispatches = [*dispatches, runtime_dispatch]
                            dispatch_by_site[address] = dispatches
                    bounded_dispatches = [dispatch for dispatch in dispatches if dispatch.bounded]
                    if bounded_dispatches:
                        for dispatch in bounded_dispatches:
                            for resolved_target in dispatch.targets:
                                add_target(resolved_target)
                                resolved_indirect[(address, resolved_target)] = IndirectTransfer(
                                    address, mnemonic, operands, resolved_target, "table", function_names.get(resolved_target, "dispatch target")
                                )
                    else:
                        row = call_site_metadata.get(address, {})
                        unresolved_calls[address] = IndirectTransfer(
                            address, mnemonic, operands, None,
                            row.get("status", "indirect/unresolved").strip(),
                            row.get("callee", "").strip(),
                        )
            add_target(next_address)
            continue

        if mnemonic in BRANCHES or mnemonic.startswith("db"):
            target = parse_direct_target(operands)
            if target is not None:
                add_target(target)
            if mnemonic not in {"bra"}:
                add_target(next_address)
            continue

        # TRAP #n enters vector 32+n. The handler can return through RTE.
        if mnemonic == "trap":
            trap_number = re.search(r"#\$?([0-9a-fA-F]+)", operands)
            if trap_number:
                slot = 0x80 + 4 * int(trap_number.group(1), 16)
                if slot + 4 <= len(rom):
                    add_target(int.from_bytes(rom[slot:slot + 4], "big"))
            add_target(next_address)
            continue

        add_target(next_address)

    reached = merge_intervals((ins.address, ins.end) for ins in instructions.values())
    exported_ranges = [
        (entry.address, min(len(rom), entry.address + (entry.size or 0)))
        for entry in functions
        if entry.size and entry.address < len(rom)
    ]
    exported_starts = {entry.address for entry in functions}
    # The status ledger has entry addresses but no sizes. Mask the entry
    # instruction only for ledger-only rows; do not invent function extents.
    ledger_entry_ranges = [
        (address, min(len(rom), address + instructions[address].size))
        for address in status_entries - exported_starts
        if address in instructions
    ]
    function_ranges = merge_intervals(exported_ranges + ledger_entry_ranges)
    outside_ranges = subtract_intervals(reached, function_ranges)
    indirect_jumps = []
    for item in listing:
        if item.mnemonic != "jmp" or not is_indirect(item.operands):
            continue
        tables = dispatch_by_site.get(item.address, [])
        targets = tuple(sorted({target for table in tables for target in table.targets}))
        indirect_jumps.append(
            IndirectJumpSite(
                item.address,
                item.operands,
                item.address in instructions,
                tables[0].base if tables else None,
                targets,
                bool(tables) and any(table.bounded for table in tables),
            )
        )
    dispatch_tables = sorted(
        (table for tables in dispatch_by_site.values() for table in tables),
        key=lambda table: (table.site, table.base, table.kind),
    )

    return Analysis(
        rom_size=len(rom),
        rom_sha256=hashlib.sha256(rom).hexdigest(),
        vector_roots=roots,
        instructions=instructions,
        resolved_indirect=sorted(resolved_indirect.values(), key=lambda item: (item.address, item.target or 0)),
        indirect_jumps=indirect_jumps,
        dispatch_tables=dispatch_tables,
        unresolved_calls=sorted(unresolved_calls.values(), key=lambda item: item.address),
        bad_targets=bad_targets,
        decode_failures=decode_failures,
        function_entries=functions,
        status_entries=status_entries,
        function_ranges=function_ranges,
        outside_ranges=outside_ranges,
    )


def fmt_addr(address: int) -> str:
    return f"0x{address:06X}"


def range_rows(ranges: Iterable[tuple[int, int]]) -> list[tuple[int, int, int]]:
    return sorted(
        ((end - start, start, end) for start, end in ranges),
        key=lambda row: (-row[0], row[1]),
    )


def build_report(analysis: Analysis) -> str:
    vector_targets = sorted({root.target for root in analysis.vector_roots})
    reached = merge_intervals((ins.address, ins.end) for ins in analysis.instructions.values())
    status_only = analysis.status_entries - {entry.address for entry in analysis.function_entries}
    largest = range_rows(analysis.outside_ranges)
    partial_functions = []
    for entry in analysis.function_entries:
        if not entry.size:
            continue
        reached_bytes = sum(
            end - start
            for start, end in intersect_intervals(
                reached, [(entry.address, entry.address + entry.size)]
            )
        )
        if 0 < reached_bytes < entry.size:
            partial_functions.append(entry)
    lines = [
        "# US ROM byte-level code reachability",
        "",
        "This is a conservative recursive-descent disassembly of ROM bytes, not a complete control-flow proof. The walk starts from valid 68000 vector targets, follows direct branches and calls, and follows indirect calls resolved by Ghidra, pointer tables, or confirmed callback records. Candidate targets from an inline indexed jump are recorded but are not followed while the input domain is unbounded. Other unresolved transfers are reported and their targets are not guessed. Data reached through an indirect edge can still decode as an instruction; review the unresolved list before using this as a completeness claim.",
        "",
        f"ROM size: **{analysis.rom_size:,} bytes**. SHA-256: `{analysis.rom_sha256}`.",
        f"Vector table: `0x000000..0x0000FF`; `{len(analysis.vector_roots)}` valid ROM vector slots resolve to `{len(vector_targets)}` unique instruction roots: {', '.join(f'`{fmt_addr(value)}`' for value in vector_targets)}.",
        "",
        "## Reachability summary",
        "",
        "| Measure | Result |",
        "|---|---:|",
        f"| Decoded reachable instruction starts | {len(analysis.instructions):,} |",
        f"| Reachable instruction bytes (union) | {analysis.reached_bytes:,} |",
        f"| Reachable bytes inside union of known function spans | {sum(end - start for start, end in intersect_intervals(reached, analysis.function_ranges)):,} |",
        f"| Known function bytes reached | **{analysis.known_function_bytes_reached:,} / {analysis.known_function_bytes_total:,}** |",
        f"| Known functions with no reached bytes | {len(analysis.unreached_functions):,} |",
        f"| Partially reached known functions | {len(partial_functions):,} |",
        f"| Reachable bytes outside known function spans | **{analysis.outside_bytes:,}** |",
        f"| Reachable ranges outside known function spans | **{len(analysis.outside_ranges):,}** |",
        f"| Exported functions used for sized spans | {len(analysis.function_entries):,} |",
        f"| Status-ledger-only entry addresses | {len(status_only):,} |",
        f"| Resolved indirect call targets followed | {len(analysis.resolved_indirect):,} |",
        f"| Pointer/dispatch sources discovered | {len(analysis.dispatch_tables):,} |",
        f"| Indirect JMP sites found in Ghidra listing | {len(analysis.indirect_jumps):,} |",
        f"| Unresolved indirect jump sites | {len(analysis.unresolved_jumps):,} |",
        f"| Reachable unresolved indirect jumps | {sum(site.reached for site in analysis.unresolved_jumps):,} |",
        f"| Unresolved indirect calls | {len(analysis.unresolved_calls):,} |",
        f"| Invalid/out-of-ROM control-flow targets | {len(analysis.bad_targets):,} |",
        f"| Vector/branch targets that Capstone could not decode | {len(analysis.decode_failures):,} |",
        "",
        "Known-function byte totals use the sum of exact `entry` and `size` values in `decompilation/export/functions.tsv`; reached bytes are counted within each function extent. The current inventory totals `111,098` bytes. The status ledger contains entry addresses but no sizes; for ledger-only entries, only the decoded entry instruction is masked as known. No body length is inferred for those rows.",
        "",
        "## Known functions with no reached bytes",
        "",
        "These exported functions have no decoded reachable byte within their sized span. Partially reached functions are counted in the summary but are not repeated here.",
        "",
        "| ROM entry | Size (bytes) | Name |",
        "|---:|---:|---|",
    ]
    if analysis.unreached_functions:
        for entry in analysis.unreached_functions:
            lines.append(f"| `{fmt_addr(entry.address)}` | {entry.size:,} | {entry.name} |")
    else:
        lines.append("| — | 0 | None |")

    lines.extend([
        "",
        "## Reachable ranges outside known functions",
        "",
        "Ranges are half-open byte intervals in the calculations; the table displays inclusive end addresses. All ranges are listed, ordered by descending size.",
        "",
        "| Rank | Start | End (inclusive) | Bytes |",
        "|---:|---:|---:|---:|",
    ])
    for rank, (size, start, end) in enumerate(largest, start=1):
        lines.append(f"| {rank} | `{fmt_addr(start)}` | `{fmt_addr(end - 1)}` | {size:,} |")
    if not largest:
        lines.append("| — | — | — | 0 |")

    lines.extend([
        "",
        "## Discovered pointer dispatch tables",
        "",
        "The analyzer recognizes direct and indexed `lea table,An` pointer loads, tables passed to shared dispatchers, active-record callbacks, and title delay/callback records. Pointer entries are read from raw ROM bytes. An explicit guard supplies a count when its compare/branch shape is recognized; otherwise entries stop at the next discovered table base or first invalid/non-code pointer. Caller-supplied tables use their ROM data boundary. The inline PC-indexed jump at `0x2510A` has a candidate block list, but its source record upper bound is not established, so those targets are not traversed. Tables are candidate target sets, not proof that every branch is dynamically reachable.",
        "",
        "| Transfer site | Kind | Table / record base | Candidate entries | Candidate code targets | ROM evidence | Target domain |",
        "|---:|---|---:|---:|---|---|---|",
    ])
    if analysis.dispatch_tables:
        for table in analysis.dispatch_tables:
            targets = ", ".join(f"`{fmt_addr(target)}`" for target in table.targets)
            base_label = (
                f"[C] ROM `{fmt_addr(table.base)}`"
                if table.base < analysis.rom_size
                else f"RAM `{fmt_addr(table.base)}`"
            )
            evidence = ", ".join(f"[C] `{fmt_addr(address)}`" for address in table.evidence)
            domain = "bounded by source structure" if table.bounded else "candidate only; input bound open"
            lines.append(
                f"| `{fmt_addr(table.site)}` | {table.kind} | {base_label} | {len(table.targets)} | {targets} | {evidence or '—'} | {domain} |"
            )
    else:
        lines.append("| — | — | — | 0 | None discovered | — | — |")

    lines.extend([
        "",
        "## Unresolved indirect jumps",
        "",
        "Indirect `JMP` instructions without a discovered table are listed here. Unreached sites are included because a preceding unresolved call can hide their callers.",
        "",
        "| Reachability | Site | Instruction | Resolution |",
        "|---|---:|---|---|",
    ])
    if analysis.unresolved_jumps:
        for site in analysis.unresolved_jumps:
            reachability = "reachable" if site.reached else "not reached from known roots"
            if site.targets:
                resolution = (
                    f"candidate inline range `[C] {fmt_addr(site.table_base or 0)}`: "
                    f"{len(site.targets)} starts; input bound not established"
                )
            else:
                resolution = "target not statically bounded"
            lines.append(f"| {reachability} | `{fmt_addr(site.address)}` | `jmp {site.operands}` | {resolution} |")
    else:
        lines.append("| — | — | None found | — |")

    lines.extend([
        "",
        "## Indirect calls",
        "",
        "Confirmed call-site targets and bounded discovered dispatch sources are traversed; unresolved calls still have their return-site fallthrough explored.",
        "",
        "| Site | Instruction | Target / status | Callee |",
        "|---:|---|---|---|",
    ])
    for transfer in analysis.resolved_indirect:
        lines.append(f"| `{fmt_addr(transfer.address)}` | `{transfer.mnemonic} {transfer.operands}` | `{fmt_addr(transfer.target or 0)}` | {transfer.callee or '—'} |")
    for transfer in analysis.unresolved_calls:
        lines.append(f"| `{fmt_addr(transfer.address)}` | `{transfer.mnemonic} {transfer.operands}` | {transfer.status} | {transfer.callee or '—'} |")
    if not analysis.resolved_indirect and not analysis.unresolved_calls:
        lines.append("| — | None reached | — | — |")

    lines.extend([
        "",
        "## Method and limits",
        "",
        "- Read the 64 longword vector slots at `0x000000..0x0000FF`; slot zero is the initial stack pointer and is not treated as code.",
        "- Decode reachable instructions from the raw big-endian ROM using Capstone's M68000 decoder. Follow direct conditional branches, DBcc branches, direct calls, and direct jumps. Calls keep their return-site fallthrough; unconditional jumps and returns do not.",
        "- Follow Ghidra `call-sites.tsv` indirect-call rows only when `status=resolved` and the target is present. Recognize indexed longword pointer loads, caller-supplied tables, ten active-effect callback slots, and title schedule records. For indexed PC jumps, derive fixed-stride candidate starts from the inline arithmetic and branch boundary but leave them unresolved when the record value has no proven upper bound. The full `disassembly.s` export inventories indirect JMP sites, including sites not reached because of an earlier unresolved call.",
        "- Merge decoded instruction bytes, subtract exported function extents and the entry instruction for status-only entries, then merge the remaining adjacent bytes into ranges.",
        "- This report does not claim completeness for indirect targets, computed dispatches without a recognized pointer table, invalid decode boundaries, or data/code ambiguities. Table entries are candidate targets and do not establish the runtime index domain.",
        "",
        f"Regenerate with `python3 decompilation/ghidra_conversion/tools/rom_reachability.py --write-report`; validate with `python3 decompilation/ghidra_conversion/tools/rom_reachability.py --check`.",
        "",
    ])
    return "\n".join(lines)


def intersect_intervals(
    left: Iterable[tuple[int, int]], right: Iterable[tuple[int, int]]
) -> list[tuple[int, int]]:
    a = merge_intervals(left)
    b = merge_intervals(right)
    intersections: list[tuple[int, int]] = []
    i = j = 0
    while i < len(a) and j < len(b):
        start = max(a[i][0], b[j][0])
        end = min(a[i][1], b[j][1])
        if end > start:
            intersections.append((start, end))
        if a[i][1] < b[j][1]:
            i += 1
        else:
            j += 1
    return intersections


def run_analysis(args: argparse.Namespace) -> tuple[Analysis, str]:
    rom = args.rom.read_bytes()
    analysis = analyze_rom(rom, args.functions, args.status, args.call_sites, args.disassembly)
    return analysis, build_report(analysis)


def main(argv: Iterable[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--write-report", action="store_true", help="regenerate the committed Markdown report")
    mode.add_argument("--check", action="store_true", help="validate inputs and compare the committed report")
    parser.add_argument("--rom", type=Path, default=DEFAULT_ROM)
    parser.add_argument("--functions", type=Path, default=DEFAULT_FUNCTIONS)
    parser.add_argument("--status", type=Path, default=DEFAULT_STATUS)
    parser.add_argument("--call-sites", type=Path, default=DEFAULT_CALL_SITES)
    parser.add_argument("--disassembly", type=Path, default=DEFAULT_DISASSEMBLY)
    parser.add_argument("--report", type=Path, default=DEFAULT_REPORT)
    args = parser.parse_args(argv)
    try:
        analysis, report = run_analysis(args)
    except (OSError, ReachabilityError) as error:
        print(f"ROM reachability analysis failed: {error}", file=sys.stderr)
        return 1

    if args.write_report:
        args.report.parent.mkdir(parents=True, exist_ok=True)
        args.report.write_text(report, encoding="utf-8")
        action = f"Wrote {args.report}"
    else:
        if not args.report.is_file():
            print(f"ROM reachability report is missing: {args.report}", file=sys.stderr)
            return 1
        if args.report.read_text(encoding="utf-8") != report:
            script_path = Path(__file__).resolve().relative_to(ROOT)
            print(f"ROM reachability report is stale: run {script_path} --write-report", file=sys.stderr)
            return 1
        action = f"Checked {args.report}"

    print(
        f"{action}: {analysis.reached_bytes:,} reachable bytes; "
        f"{analysis.outside_bytes:,} outside known function spans in "
        f"{len(analysis.outside_ranges):,} ranges; "
        f"{sum(site.reached for site in analysis.unresolved_jumps)} reachable unresolved indirect jumps "
        f"({len(analysis.unresolved_jumps)} sites found in Ghidra) and "
        f"{len(analysis.unresolved_calls)} unresolved indirect calls."
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
