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
MAX_REPORTED_RANGES = 25

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
    def unresolved_jumps(self) -> list[IndirectJumpSite]:
        return [site for site in self.indirect_jumps if not site.targets]


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


def pointer_load_register(operands: str) -> str | None:
    match = re.fullmatch(r"\(\s*(a[0-7])\s*\)\s*,\s*(a[0-7])", operands, re.IGNORECASE)
    if not match or match.group(1).lower() != match.group(2).lower():
        return None
    return match.group(1).lower()


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
        # inclusive index to [0, immediate].
        if branch_mnemonic in {"bgt", "bhi"} and target > site:
            return immediate + 1
    return None


def discover_dispatch_tables(
    rom: bytes,
    listing: list[ListingInstruction],
    function_starts: set[int],
) -> dict[int, DispatchTable]:
    """Resolve simple absolute longword dispatch tables used through An.

    The supported shape is `lea table,An`, optional address arithmetic,
    `movea.l (An),An`, then `jmp/jsr (An)`. A nearby explicit index guard may
    supply the table length; otherwise entries must point at Ghidra-listed
    code starts. A table ends at the next separately referenced table base or
    its first invalid entry.
    """
    indexes = {item.address: index for index, item in enumerate(listing)}
    transfer_bases: dict[int, tuple[str, int]] = {}
    table_bases: set[int] = set()
    for item in listing:
        if item.mnemonic not in {"jmp", "jsr"}:
            continue
        register = indirect_register(item.operands)
        if not register:
            continue
        index = indexes[item.address]
        lower_bound = max((start for start in function_starts if start <= item.address), default=0)
        first = max(0, index - 32)
        for load_index in range(index - 1, first - 1, -1):
            load = listing[load_index]
            if load.address < lower_bound:
                break
            if load.mnemonic != "movea.l" or pointer_load_register(load.operands) != register:
                continue
            for lea_index in range(load_index - 1, first - 1, -1):
                lea = listing[lea_index]
                if lea.address < lower_bound:
                    break
                if lea.mnemonic not in {"lea", "lea.l"}:
                    continue
                base = absolute_lea(lea.operands)
                if base and base[1] == register:
                    transfer_bases[item.address] = (item.mnemonic, base[0])
                    table_bases.add(base[0])
                    break
            if item.address in transfer_bases:
                break

    if not transfer_bases:
        return {}

    known_code_starts = {item.address for item in listing} | function_starts
    disassembler = Cs(CS_ARCH_M68K, CS_MODE_M68K_000 | CS_MODE_BIG_ENDIAN)
    ordered_bases = sorted(table_bases)
    targets_by_base: dict[int, tuple[int, ...]] = {}
    counts_by_base: dict[int, int] = {}
    for site, (_, base) in transfer_bases.items():
        count = guarded_table_count(listing, site, base)
        if count is not None:
            counts_by_base[base] = max(counts_by_base.get(base, 0), count)
    for base_index, base in enumerate(ordered_bases):
        upper = ordered_bases[base_index + 1] if base_index + 1 < len(ordered_bases) else min(len(rom), base + 4 * 64)
        count_limit = counts_by_base.get(base)
        if count_limit is not None:
            upper = min(upper, base + 4 * count_limit)
        targets: list[int] = []
        for entry in range(base, min(upper, len(rom) - 3), 4):
            target = int.from_bytes(rom[entry:entry + 4], "big")
            if target >= len(rom) or target & 1:
                break
            if count_limit is None:
                if target not in known_code_starts:
                    break
            elif decode_one(disassembler, rom, target) is None:
                break
            targets.append(target)
        targets_by_base[base] = tuple(targets)

    return {
        site: DispatchTable(site, kind, base, targets_by_base[base])
        for site, (kind, base) in transfer_bases.items()
        if targets_by_base.get(base)
    }


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
                dispatch = dispatch_by_site.get(address)
                if dispatch:
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
                    dispatch = dispatch_by_site.get(address)
                    if dispatch:
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
    indirect_jumps = [
        IndirectJumpSite(
            item.address,
            item.operands,
            item.address in instructions,
            dispatch_by_site[item.address].base if item.address in dispatch_by_site else None,
            dispatch_by_site[item.address].targets if item.address in dispatch_by_site else (),
        )
        for item in listing
        if item.mnemonic == "jmp" and is_indirect(item.operands)
    ]
    dispatch_tables = sorted(dispatch_by_site.values(), key=lambda table: (table.site, table.base))

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
    lines = [
        "# US ROM byte-level code reachability",
        "",
        "This is a conservative recursive-descent disassembly of ROM bytes, not a complete control-flow proof. The walk starts from valid 68000 vector targets, follows direct branches and calls, follows indirect calls resolved by Ghidra or a discovered absolute pointer table, and follows discovered indirect jump tables. Other unresolved transfers are reported and their targets are not guessed. Data that is reached through an indirect edge can still decode as an instruction; review the unresolved list before using this as a completeness claim.",
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
        f"| Reachable bytes inside known function spans | {sum(end - start for start, end in intersect_intervals(reached, analysis.function_ranges)):,} |",
        f"| Reachable bytes outside known function spans | **{analysis.outside_bytes:,}** |",
        f"| Reachable ranges outside known function spans | **{len(analysis.outside_ranges):,}** |",
        f"| Exported functions used for sized spans | {len(analysis.function_entries):,} |",
        f"| Status-ledger-only entry addresses | {len(status_only):,} |",
        f"| Resolved indirect call targets followed | {len(analysis.resolved_indirect):,} |",
        f"| Absolute pointer dispatch tables discovered | {len(analysis.dispatch_tables):,} |",
        f"| Indirect JMP sites found in Ghidra listing | {len(analysis.indirect_jumps):,} |",
        f"| Unresolved indirect jump sites | {len(analysis.unresolved_jumps):,} |",
        f"| Reachable unresolved indirect jumps | {sum(site.reached for site in analysis.unresolved_jumps):,} |",
        f"| Unresolved indirect calls | {len(analysis.unresolved_calls):,} |",
        f"| Invalid/out-of-ROM control-flow targets | {len(analysis.bad_targets):,} |",
        f"| Vector/branch targets that Capstone could not decode | {len(analysis.decode_failures):,} |",
        "",
        "Known function spans use the exact `entry` and `size` values in `decompilation/export/functions.tsv`. The status ledger contains entry addresses but no sizes; for ledger-only entries, only the decoded entry instruction is masked as known. No body length is inferred for those rows.",
        "",
        "## Largest reachable ranges outside known functions",
        "",
        "Ranges are half-open byte intervals in the calculations; the table displays inclusive end addresses.",
        "",
        "| Rank | Start | End (inclusive) | Bytes |",
        "|---:|---:|---:|---:|",
    ]
    for rank, (size, start, end) in enumerate(largest[:MAX_REPORTED_RANGES], start=1):
        lines.append(f"| {rank} | `{fmt_addr(start)}` | `{fmt_addr(end - 1)}` | {size:,} |")
    if not largest:
        lines.append("| — | — | — | 0 |")

    lines.extend([
        "",
        "## Discovered pointer dispatch tables",
        "",
        "The analyzer recognizes the `lea table,An` → `movea.l (An),An` → indirect transfer shape and reads consecutive big-endian longword entries. A nearby explicit ROM index guard bounds a table when its compare and branch shape is recognized; those entries are accepted when the raw ROM bytes decode as a 68000 instruction, even if Ghidra has no listing row there. Without a recognized guard, entries must point to Ghidra-listed code starts and stop at the next discovered table base or first invalid pointer. These remain candidate targets, not a proof that every path is reachable.",
        "",
        "| Transfer site | Kind | Table base | Candidate entries | Candidate code targets |",
        "|---:|---|---:|---:|---|",
    ])
    if analysis.dispatch_tables:
        for table in analysis.dispatch_tables:
            targets = ", ".join(f"`{fmt_addr(target)}`" for target in table.targets)
            lines.append(f"| `{fmt_addr(table.site)}` | {table.kind} | `{fmt_addr(table.base)}` | {len(table.targets)} | {targets} |")
    else:
        lines.append("| — | — | — | 0 | None discovered |")

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
            lines.append(f"| {reachability} | `{fmt_addr(site.address)}` | `jmp {site.operands}` | target not statically bounded |")
    else:
        lines.append("| — | — | None found | — |")

    lines.extend([
        "",
        "## Indirect calls",
        "",
        "Confirmed call-site targets and discovered pointer-table entries are traversed; unresolved calls still have their return-site fallthrough explored.",
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
        "- Follow Ghidra `call-sites.tsv` indirect-call rows only when `status=resolved` and the target is present. For simple absolute pointer tables, recognize `lea table,An` followed by `movea.l (An),An` and an indirect `jmp/jsr`. Read candidate longword targets to the count implied by a nearby compare-and-branch index guard when recognized (greater-than skip: inclusive maximum plus one; less-than branch into dispatch: comparison immediate); otherwise stop at the next discovered table base or first non-listed pointer. The full `disassembly.s` export inventories indirect JMP sites, including sites not reached because of an earlier unresolved call.",
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
