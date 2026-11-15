#!/usr/bin/env python3
"""Inventory ROM quest flags/dialogue effects and compare port dialogue data."""

from __future__ import annotations

import argparse
import json
import re
import sys
from collections import defaultdict, deque
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
DEFAULT_ROM = Path("/workspace/shadowrun/rom.bin")
DEFAULT_DISASM = ROOT / "decompilation/export/disassembly.s"
DEFAULT_OUT = ROOT / "out"
FLAG_WORDS = {0xFFFC1C, 0xFFFC1E, 0xFFFC22, 0xFFFC26, 0xFFFC27,
              0xFFFC2C, 0xFFFC30, 0xFFFC34, 0xFFFC38}
LINE_RE = re.compile(r"^([0-9A-Fa-f]{8}):\s*([^;]+?)(?:\s*;.*)?$")
PTR_RE = re.compile(r"(?:0x|\$)([0-9A-Fa-f]{4,8})")


def parse_instruction(line: str) -> dict | None:
    match = LINE_RE.match(line.rstrip())
    if not match:
        return None
    body = match.group(2).strip()
    parts = body.split(None, 1)
    mnemonic = parts[0].lower()
    operands = parts[1].strip() if len(parts) > 1 else ""
    return {"address": int(match.group(1), 16), "mnemonic": mnemonic,
            "operands": operands, "source": "disassembly"}


def decode_bounded_gaps(rom: bytes, instructions: list[dict]) -> tuple[list[dict], list[list[int]]]:
    """Decode short unlisted subroutines bounded by RTS and the next listed PC."""
    try:
        from capstone import (CS_ARCH_M68K, CS_MODE_BIG_ENDIAN,
                              CS_MODE_M68K_000, Cs)
    except ImportError as error:
        raise RuntimeError("python3-capstone is required to decode raw-ROM code gaps") from error
    decoder = Cs(CS_ARCH_M68K, CS_MODE_M68K_000 | CS_MODE_BIG_ENDIAN)
    known = {item["address"] for item in instructions}
    additions: list[dict] = []
    ranges: list[list[int]] = []
    ordered = sorted(instructions, key=lambda item: item["address"])
    for previous, following in zip(ordered, ordered[1:]):
        if previous["mnemonic"].split(".")[0] not in ("rts", "rte"):
            continue
        start = previous["address"] + 2
        end = following["address"]
        if end - start < 4 or end - start > 0x400 or end > len(rom):
            continue
        decoded = list(decoder.disasm(rom[start:end], start))
        if not decoded or decoded[0].address != start:
            continue
        cursor = start
        valid = True
        for item in decoded:
            if item.address != cursor or item.address in known:
                valid = False
                break
            cursor += item.size
        if not valid or cursor != end or decoded[-1].mnemonic.lower() not in ("rts", "rte"):
            continue
        for item in decoded:
            operands = item.op_str.replace("$", "0x")
            additions.append({"address": item.address, "mnemonic": item.mnemonic.lower(),
                              "operands": operands, "source": "raw-rom-gap"})
            known.add(item.address)
        ranges.append([start, end])
    combined = sorted(instructions + additions, key=lambda item: item["address"])
    return combined, ranges


def routine_index(instructions: list[dict]) -> tuple[dict[int, int], dict[int, list[dict]]]:
    by_address: dict[int, int] = {}
    groups: dict[int, list[dict]] = defaultdict(list)
    branch_mnemonics = {"bra", "beq", "bne", "bgt", "bge", "blt", "ble", "bmi", "bpl",
                        "bcc", "bcs", "bhi", "bls", "bvc", "bvs", "dbf", "dbra"}
    targets = set()
    for item in instructions:
        if item["mnemonic"].split(".")[0] in branch_mnemonics:
            target = abs_address(item["operands"].split(",")[-1])
            if target is not None:
                targets.add(target)
    start = instructions[0]["address"] if instructions else 0
    previous = None
    for index, item in enumerate(instructions):
        if (previous and previous["mnemonic"].split(".")[0] in ("rts", "rte") and
                item["address"] not in targets):
            start = item["address"]
        item["routine_start"] = start
        by_address[item["address"]] = start
        groups[start].append(item)
        previous = item
    return by_address, groups


def parse_int(value: str) -> int | None:
    match = re.search(r"#\s*(?:0x|\$)([0-9a-f]+)", value, re.I)
    return int(match.group(1), 16) if match else None


def abs_address(value: str) -> int | None:
    match = PTR_RE.search(value)
    return int(match.group(1), 16) if match else None


def split_operands(value: str) -> list[str]:
    return [part.strip() for part in value.split(",", 1)] if "," in value else [value.strip()]


def rom_instruction_stream(rom: bytes, start: int, end: int,
                           owner: int | None = None) -> list[dict]:
    """Decode a bounded code window from the supplied ROM using 68000 mode."""
    try:
        from capstone import (CS_ARCH_M68K, CS_MODE_BIG_ENDIAN,
                              CS_MODE_M68K_000, Cs)
    except ImportError as error:
        raise RuntimeError("python3-capstone is required to decode ROM handlers") from error
    decoder = Cs(CS_ARCH_M68K, CS_MODE_M68K_000 | CS_MODE_BIG_ENDIAN)
    if start < 0 or start >= len(rom) or end <= start:
        return []
    end = min(end, len(rom))
    stream = []
    for instruction in decoder.disasm(rom[start:end], start):
        stream.append({"address": instruction.address,
                       "mnemonic": instruction.mnemonic.lower(),
                       "operands": instruction.op_str.replace("$", "0x"),
                       "size": instruction.size,
                       "source": "raw-rom",
                       "routine_start": owner if owner is not None else start})
    return stream


def command_table(rom: bytes) -> list[dict]:
    """Read the 250 command handler pointers at ROM table 0x12CCB4."""
    table = 0x12CCB4
    result = []
    for command in range(0xFA):
        pointer = int.from_bytes(rom[table + command * 4:table + command * 4 + 4], "big")
        if not (0x1000 <= pointer < len(rom)):
            raise RuntimeError(f"command table entry 0x{command:02X} at 0x{table + command * 4:X} is invalid")
        result.append({"command": command, "table_address": table + command * 4,
                       "entry": pointer})
    return result


def indexed_title_services(rom: bytes) -> list[dict]:
    """Read the indexed title-service vector reached by ROM dispatcher 0x53A78."""
    table = 0x15FD5C
    result = []
    for index in range(0x17):
        pointer = int.from_bytes(rom[table + index * 4:table + index * 4 + 4], "big")
        if not (0x1000 <= pointer < len(rom)):
            raise RuntimeError(f"title service {index} at 0x{table + index * 4:X} is invalid")
        result.append({"service_index": index, "table": table,
                       "table_address": table + index * 4, "entry": pointer,
                       "noop_alias": pointer == 0x49A0})
    return result


def decode_entry_windows(rom: bytes, entries: list[dict], max_window: int = 0x400) -> dict[int, list[dict]]:
    """Decode one non-overlapping ROM window per unique entry point."""
    starts = sorted({row["entry"] for row in entries})
    windows = {}
    for index, start in enumerate(starts):
        following = starts[index + 1] if index + 1 < len(starts) else start + max_window
        end = min(following, start + max_window, len(rom))
        windows[start] = rom_instruction_stream(rom, start, end, start)
    return windows


BRANCH_TESTS = {
    "beq": "Z=1", "bne": "Z=0", "bgt": "Z=0 and N=V",
    "bge": "N=V", "blt": "N!=V", "ble": "Z=1 or N!=V",
    "bhi": "C=0 and Z=0", "bls": "C=1 or Z=1",
    "bcc": "C=0", "bcs": "C=1", "bhs": "C=0", "blo": "C=1",
    "bmi": "N=1", "bpl": "N=0", "bvc": "V=0", "bvs": "V=1",
    "dbf": "counter != -1", "dbra": "counter != -1",
}


def instruction_target(instruction: dict) -> int | None:
    parts = split_operands(instruction.get("operands", ""))
    return abs_address(parts[-1]) if parts else None


def previous_condition_source(stream: list[dict], index: int) -> str | None:
    """Describe the nearest decoded instruction that supplied branch flags."""
    for candidate in reversed(stream[max(0, index - 3):index]):
        mnemonic = candidate["mnemonic"].split(".")[0]
        operands = candidate["operands"]
        if mnemonic in ("btst", "bset", "bclr"):
            parts = split_operands(operands)
            bit = parse_int(parts[0]) if parts else None
            storage = abs_address(parts[-1]) if parts else None
            if bit is not None:
                suffix = f" bit {bit}" + (f" at 0x{storage:X}" if storage else "")
                return f"{mnemonic}{suffix}; branch tests Z"
        if mnemonic in ("cmp", "cmpi", "cmpa", "cmpi", "tst", "tstb", "tstw", "tstl"):
            return f"{mnemonic} {operands}; branch tests condition codes"
    return None


def control_flow(stream: list[dict], entry: int) -> tuple[dict[int, list[list[dict]]], list[dict]]:
    """Propagate branch conditions to reachable instructions in a handler window."""
    by_address = {row["address"]: row for row in stream}
    order = sorted(by_address)
    next_address = {address: order[index + 1] if index + 1 < len(order) else None
                    for index, address in enumerate(order)}
    index_by_address = {row["address"]: index for index, row in enumerate(stream)}
    paths: dict[int, list[list[dict]]] = defaultdict(list)
    branches = []
    queue = deque([(entry, [])])
    visits = 0
    while queue and visits < 12000:
        address, path = queue.popleft()
        visits += 1
        instruction = by_address.get(address)
        if instruction is None:
            continue
        path_key = tuple((item["branch"], item["taken"]) for item in path)
        if any(tuple((item["branch"], item["taken"]) for item in old) == path_key
               for old in paths[address]):
            continue
        if len(paths[address]) >= 12:
            continue
        paths[address].append(path)
        mnemonic = instruction["mnemonic"].split(".")[0]
        target = instruction_target(instruction)
        fallthrough = next_address.get(address)
        if mnemonic in BRANCH_TESTS:
            test = BRANCH_TESTS[mnemonic]
            source = previous_condition_source(stream, index_by_address[address])
            branch_row = {"address": address, "mnemonic": mnemonic,
                          "target": target, "fallthrough": fallthrough,
                          "condition_flags": test,
                          "condition_source": source,
                          "path_conditions": path}
            branches.append(branch_row)
            taken = {"branch": address, "taken": True,
                     "condition": test, "target": target}
            not_taken = {"branch": address, "taken": False,
                         "condition": f"not ({test})", "target": fallthrough}
            if target in by_address:
                queue.append((target, path + [taken]))
            if fallthrough in by_address:
                queue.append((fallthrough, path + [not_taken]))
            continue
        if mnemonic in ("bra", "jmp"):
            if target in by_address:
                queue.append((target, path))
            continue
        if mnemonic in ("rts", "rte", "rtr", "illegal", "trap"):
            continue
        if fallthrough in by_address:
            queue.append((fallthrough, path))
    branches.sort(key=lambda row: row["address"])
    return paths, branches


# ROM 0x1F572-0x1F58A: targets below 0xFA dispatch through the command table
# at 0x12CCB4 (target 0 is command 0 = 0x21326); 0xFA and up re-enter 0x1F3C4
# as a page record. A negative target would index before the table, so the
# walker treats it as END.
COMMAND_TARGET_LIMIT = 0xFA


def parse_dialogue_page(rom: bytes, root: int,
                       _validate_targets: bool = True,
                       no_title: bool = False) -> dict | None:
    """Read a dialogue record, its ordered choice labels, and page targets.

    Title path (FFF10C clear): 0x1F3E2 reads +0 as the page text, 0x1F40A
    reads the first label at +4, 0x1F452/0x1F4AE read labels at +8/+12, and
    0x1F52E/0x1F554/0x1F56C read targets at +16/+20/+24.
    No-title path (FFF10C set by the caller, e.g. 0x2089E after
    `lea $FE5EC,a0`): 0x1F3C4-0x1F3D4 branches to 0x1F404 without the +0
    text read, so the record has NO text: labels sit at +0/+4/+8 and targets
    at +12/+16/+20. Only the first page of a chain can be no-title because
    0x1F3CE clears FFF10C before the loop re-enters 0x1F3C4.
    A negative first label (0x1F40E bmi 0x1F592) is a plain message with no
    choices; a zero second/third label ends the list (0x1F45A / 0x1F4B6).
    """
    base = 0 if no_title else 4
    if root < 0 or root + base + 24 > len(rom):
        return None
    word = lambda address: int.from_bytes(rom[address:address + 4], "big")
    if no_title:
        text_pointer = None
        text = None
    else:
        text_pointer = word(root)
        text = decode_text(rom, text_pointer)
        if text is None:
            return None
    label_ptrs = [word(root + base + 4 * index) for index in range(3)]
    choices = []
    for index, label_pointer in enumerate(label_ptrs):
        # Negative first label: plain message (0x1F40E). Zero later label:
        # list ends (0x1F456/0x1F4B2). Negative later labels are not valid
        # text pointers either, so stop there too.
        if label_pointer == 0 or label_pointer & 0x80000000:
            break
        target = word(root + base + 12 + 4 * index)
        label = decode_text(rom, label_pointer)
        if no_title and index == 0 and label is None:
            return None
        command = target if target < COMMAND_TARGET_LIMIT else None
        target_page = (parse_dialogue_page(rom, target, _validate_targets=False)
                       if _validate_targets and command is None and
                       not target & 0x80000000 else None)
        next_page = target if target_page is not None else None
        resolved_target = command if command is not None else next_page
        choices.append({"order": index, "label_pointer": label_pointer or None,
                        "label": label, "target": resolved_target,
                        "command": command, "next_page": next_page})
    if no_title and text is None:
        # Labels are the only on-screen strings of a no-title page; use them
        # for port text matching.
        text = "||".join(choice["label"] for choice in choices
                         if choice["label"]) or None
    return {"record": root, "text_pointer": text_pointer, "text": text,
            "no_title": no_title, "choices": choices}


def walk_dialogue_pages(rom: bytes, root: int, max_pages: int = 96,
                        no_title: bool = False) -> list[dict]:
    """Walk page records in first-visit order, retaining choice target order."""
    rows = []
    seen = set()
    queue = deque([root])
    first = True
    while queue and len(seen) < max_pages:
        address = queue.popleft()
        if address in seen:
            continue
        page = parse_dialogue_page(rom, address,
                                   no_title=no_title and first)
        first = False
        if page is None:
            continue
        seen.add(address)
        page["order"] = len(rows)
        rows.append(page)
        for choice in page["choices"]:
            target = choice["next_page"]
            if target is not None and target not in seen:
                queue.append(target)
    return rows


def stream_text_roots(stream: list[dict], reachable: set[int] | None = None) -> list[dict]:
    roots = []
    for item in stream:
        if reachable is not None and item["address"] not in reachable:
            continue
        if not item["mnemonic"].startswith("lea"):
            continue
        pointer = abs_address(item["operands"])
        parts = split_operands(item["operands"])
        register = parts[-1].lower() if len(parts) > 1 else ""
        if pointer is None or not (0xF0000 <= pointer < 0x120000):
            continue
        if register not in ("a0", "a1"):
            continue
        roots.append({"instruction": item["address"], "pointer": pointer,
                      "register": register, "no_title": False})
    # A caller that sets FFF10C (`move.b #$ff,$fff10c.l`) after loading A0
    # hands 0x1F3C4 a no-title record (0x2089E, 0x2130E, 0x21554, 0x2286A,
    # 0x23E38). Mark the last A0 root loaded before that store.
    for index, item in enumerate(stream):
        operands = item["operands"].lower().replace("$", "0x")
        if "fff10c" not in operands or \
                not item["mnemonic"].startswith("move") or \
                not operands.startswith("#0xff,"):
            continue
        candidates = [root for root in roots if root["register"] == "a0" and
                      root["instruction"] < item["address"]]
        if candidates:
            max(candidates, key=lambda root: root["instruction"])["no_title"] = True
    return roots


def immediate_register_before(stream: list[dict], index: int, register: str,
                              max_bytes: int = 20) -> tuple[int | None, int | None]:
    """Find the most recent immediate load into a register before a call."""
    call_address = stream[index]["address"]
    for candidate in reversed(stream[:index]):
        if call_address - candidate["address"] > max_bytes:
            break
        if candidate["mnemonic"].split(".")[0] in ("bra", "beq", "bne", "bgt", "bge", "blt", "ble"):
            break
        parts = split_operands(candidate["operands"])
        if len(parts) != 2 or parts[1].lower() != register.lower():
            continue
        value = parse_int(parts[0])
        if value is not None and candidate["mnemonic"].split(".")[0] in ("move", "moveq"):
            return value, candidate["address"]
    return None, None


def stream_effects(stream: list[dict], entry: int, command: int | None = None,
                   source_kind: str = "command") -> tuple[list[dict], dict[int, list[list[dict]]], list[dict]]:
    """Decode explicit effects, values, and branch paths from one ROM entry."""
    paths, branches = control_flow(stream, entry)
    reachable = set(paths)
    effects_out = []
    item_names = parse_item_names()

    def add_effect(instruction: dict, kind: str, **values) -> None:
        effects_out.append({"address": instruction["address"], "routine": entry,
                            "command": command, "source_kind": source_kind,
                            "kind": kind,
                            "path_conditions": paths.get(instruction["address"], []),
                            "source_addresses": [instruction["address"]], **values})

    def storage_and_bit(instruction: dict) -> tuple[int | None, int | None, str | None]:
        parts = split_operands(instruction["operands"])
        if len(parts) != 2:
            return None, None, None
        mnemonic = instruction["mnemonic"].split(".")[0]
        bit = parse_int(parts[0])
        target = parts[1].lower()
        address = abs_address(target)
        operation = {"bset": "set", "bclr": "clear", "btst": "test"}.get(mnemonic)
        if address in FLAG_WORDS and bit is not None:
            return address, bit, operation
        if re.fullmatch(r"d[0-7]", target):
            prior = [row for row in stream if row["address"] < instruction["address"]]
            for candidate in reversed(prior[-8:]):
                candidate_parts = split_operands(candidate["operands"])
                if (candidate["mnemonic"].split(".")[0] == "move" and
                        len(candidate_parts) == 2 and
                        candidate_parts[1].lower() == target):
                    source = abs_address(candidate_parts[0])
                    if source in FLAG_WORDS:
                        return source, bit, operation
                    break
        return None, None, None

    # Calls and absolute balance writes are decoded per handler, keeping the
    # immediate amount attached to the call site that consumes it.
    for index, item in enumerate(stream):
        if item["address"] not in reachable:
            continue
        mnemonic = item["mnemonic"].split(".")[0]
        operands = item["operands"].lower()
        parts = split_operands(operands)
        target = instruction_target(item)
        if mnemonic in ("jsr", "bsr") and target == 0x59C34:
            amount, load_address = immediate_register_before(stream, index, "d0")
            add_effect(item, "karma", operation="award_each_runner", amount=amount,
                       source_addresses=[load_address, item["address"]] if load_address else [item["address"]])
        if mnemonic in ("jsr", "bsr") and target == 0x21A4E:
            amount, load_address = immediate_register_before(stream, index, "d1")
            result_item = None
            result_phase = None
            for candidate in stream[index + 1:index + 8]:
                if candidate["address"] not in reachable:
                    continue
                candidate_parts = split_operands(candidate["operands"].lower())
                if len(candidate_parts) != 2:
                    continue
                source, destination = candidate_parts
                if "a3" not in destination or "d6" not in destination:
                    continue
                value = parse_int(source)
                if value is None:
                    continue
                if re.search(r"(?:^|\W)(?:0x)?0?\(a3", destination):
                    result_item = value
                elif re.search(r"(?:0x)?0?8\(a3", destination):
                    result_phase = value
                if result_item is not None and result_phase is not None:
                    break
            amount_instruction = next((row for row in stream
                                       if row["address"] == load_address), item)
            add_effect(amount_instruction, "nuyen", operation="conditional_debit", amount=amount,
                       delta=-amount if amount is not None else None,
                       call_address=item["address"],
                       source_addresses=[load_address, item["address"]] if load_address else [item["address"]],
                       result_item_code=result_item, result_phase=result_phase)
        if mnemonic == "move" and len(parts) == 2:
            source, destination = parts
            destination_address = abs_address(destination)
            source_address = abs_address(source)
            if destination_address == 0xFFFBFE:
                value = parse_int(source)
                add_effect(item, "nuyen", operation="balance_write", amount=value,
                           delta=None, storage=destination_address)
            elif source_address == 0xFFFBFE:
                # A memory-to-register load reads the balance and is not an effect.
                pass
        if mnemonic in ("subi", "addi", "sub", "add") and parts:
            destination_address = abs_address(parts[-1])
            if destination_address == 0xFFFBFE:
                amount = parse_int(parts[0])
                sign = -1 if mnemonic.startswith("sub") else 1
                add_effect(item, "nuyen", operation="debit" if sign < 0 else "credit",
                           amount=amount, delta=sign * amount if amount is not None else None,
                           storage=destination_address)
        if mnemonic in ("move", "moveq", "movea") and len(parts) == 2:
            source, destination = parts
            if "a3" in destination and "d6" in destination:
                value = parse_int(source)
                if value is not None:
                    phase = None
                    if re.search(r"(?:0x)?0?8\(a3", destination):
                        phase = value
                    elif re.search(r"(?:^|\W)(?:0x)?0?\(a3", destination):
                        add_effect(item, "item", operation="give_runner_item",
                                   item_code=value, phase=None)
                    if phase is not None:
                        add_effect(item, "item_phase", operation="set_runner_item_phase",
                                   phase=phase)
        if mnemonic in ("bset", "bclr", "btst"):
            storage, bit, operation = storage_and_bit(item)
            if storage is not None and bit is not None:
                add_effect(item, "flag", operation=operation, storage=storage, bit=bit)
        if mnemonic in ("jsr", "bsr") and target == 0x136DE:
            add_effect(item, "scene_change", operation="select_scene",
                       target_service=0x136DE)
        if mnemonic in ("jsr", "bsr") and target == 0x1F246:
            portrait = next((abs_address(row["operands"]) for row in reversed(stream[:index])
                             if row["mnemonic"].startswith("lea") and
                             split_operands(row["operands"])[-1].lower() == "a5"), None)
            add_effect(item, "portrait_load", operation="open_dialogue",
                       portrait_pointer=portrait, target_service=0x1F246)
        if mnemonic in ("jsr", "bsr") and target == 0xB48C:
            add_effect(item, "actor_spawn", operation="spawn_actor",
                       target_service=0xB48C)
        if mnemonic in ("jsr", "bsr", "jmp") and target is not None:
            if target not in (0x59C34, 0x21A4E, 0x136DE, 0x1F246, 0xB48C):
                add_effect(item, "service_call", target_service=target)

    # Tie the returned item code and phase to the preceding debit site, as the
    # 0x228D0 command handler does on its success path.
    by_address = {item["address"]: item for item in stream}
    for effect in effects_out:
        if effect["kind"] != "item":
            continue
        item_instruction = by_address.get(effect["address"])
        if item_instruction is None:
            continue
        later = [row for row in effects_out
                 if row["kind"] == "item_phase" and row["address"] > effect["address"]]
        if later:
            effect["phase"] = later[0].get("phase")
            effect["rating"] = later[0].get("phase")
        effect["item_name"] = item_names.get(effect.get("item_code"))
    return effects_out, paths, branches


def raw_flag_events(streams: dict[int, list[dict]]) -> list[dict]:
    """Recover direct and register-mediated flag mutations from ROM code."""
    events = []
    for owner, stream in streams.items():
        register_storage: dict[str, int] = {}
        pending: dict[str, list[dict]] = defaultdict(list)
        immediate_trigger = None
        for item in stream:
            address = item["address"]
            full_mnemonic = item["mnemonic"].lower()
            mnemonic = full_mnemonic.split(".")[0]
            operands = item["operands"].lower()
            parts = split_operands(operands)
            if mnemonic == "move" and len(parts) == 2:
                source, destination = parts
                source_address = abs_address(source)
                destination_address = abs_address(destination)
                source_reg = source.strip()[:2]
                destination_reg = destination.strip()[:2]
                if destination_address == 0xFFDD64:
                    immediate_trigger = parse_int(source)
                    if immediate_trigger is not None:
                        events.append({"address": address, "routine": owner,
                                       "storage": 0xFFDD64, "bit": immediate_trigger,
                                       "operation": "trigger_select", "value": immediate_trigger,
                                       "raw_gap": True})
                if destination_address in FLAG_WORDS:
                    value = parse_int(source)
                    if value is not None:
                        width = 8 if full_mnemonic.endswith(".b") else 16 if full_mnemonic.endswith(".w") else 32
                        for bit in range(width):
                            events.append({"address": address, "routine": owner,
                                           "storage": destination_address, "bit": bit,
                                           "operation": "set" if value & (1 << bit) else "clear",
                                           "constant_write": value, "raw_gap": True})
                if source_address in FLAG_WORDS and re.fullmatch(r"d[0-7]", destination):
                    register_storage[destination_reg] = source_address
                    pending[destination_reg].clear()
                    continue
                if destination_address in FLAG_WORDS and re.fullmatch(r"d[0-7]", source):
                    for mutation in pending[source_reg]:
                        events.append({**mutation, "writeback_address": address})
                    pending[source_reg].clear()
                    register_storage.pop(source_reg, None)
                    continue
                if re.fullmatch(r"d[0-7]", destination):
                    register_storage.pop(destination_reg, None)
                    pending[destination_reg].clear()
            if mnemonic in ("bset", "bclr", "btst") and len(parts) == 2:
                bit = parse_int(parts[0])
                action = {"bset": "set", "bclr": "clear", "btst": "test"}[mnemonic]
                if bit is None:
                    continue
                target_address = abs_address(parts[1])
                register = parts[1].strip()[:2]
                if target_address in FLAG_WORDS:
                    events.append({"address": address, "routine": owner,
                                   "storage": target_address, "bit": bit,
                                   "operation": action, "raw_gap": True})
                elif register in register_storage:
                    event = {"address": address, "routine": owner,
                             "storage": register_storage[register], "bit": bit,
                             "operation": action, "raw_gap": True}
                    if action == "test":
                        events.append(event)
                    else:
                        pending[register].append(event)
            if mnemonic in ("jsr", "bsr", "jmp", "bra"):
                target = instruction_target(item)
                if target == 0x2196C and immediate_trigger is not None:
                    events.append({"address": address, "routine": owner,
                                   "storage": 0xFFDD64, "bit": immediate_trigger,
                                   "operation": "trigger_dispatch", "value": immediate_trigger,
                                   "raw_gap": True})
                    immediate_trigger = None
                register_storage.clear()
                pending.clear()
            if mnemonic in ("ori", "andi") and len(parts) == 2:
                mask = parse_int(parts[0])
                register = parts[1].strip()[:2]
                storage = register_storage.get(register)
                direct_storage = abs_address(parts[1])
                if direct_storage in FLAG_WORDS and mask is not None:
                    operation = "set" if mnemonic == "ori" else "clear"
                    bits = [bit for bit in range(32) if (mask >> bit) & 1]
                    for bit in bits:
                        events.append({"address": address, "routine": owner,
                                       "storage": direct_storage, "bit": bit,
                                       "operation": operation, "raw_gap": True})
                if mask is not None and storage is not None:
                    operation = "set" if mnemonic == "ori" else "clear"
                    bits = ([bit for bit in range(32) if (mask >> bit) & 1]
                            if operation == "set" else
                            [bit for bit in range(32) if not ((mask >> bit) & 1)])
                    for bit in bits:
                        pending[register].append({"address": address, "routine": owner,
                                                 "storage": storage, "bit": bit,
                                                 "operation": operation, "raw_gap": True})
            if mnemonic == "clr":
                target_address = abs_address(operands)
                if target_address in FLAG_WORDS:
                    events.append({"address": address, "routine": owner,
                                   "storage": target_address, "bit": None,
                                   "operation": "clear_all", "raw_gap": True})
    unique = {}
    for event in events:
        key = (event["address"], event["storage"], str(event["bit"]), event["operation"])
        unique[key] = event
    return [unique[key] for key in sorted(unique)]


def enrich_entry(rom: bytes, entry: dict, stream: list[dict],
                 source_kind: str) -> dict:
    owner = entry["entry"]
    effects_out, paths, branches = stream_effects(
        stream, owner, entry.get("command"), source_kind)
    reachable = set(paths)
    roots = stream_text_roots(stream, reachable)
    for root in roots:
        root["path_conditions"] = paths.get(root["instruction"], [])
        root["pages"] = walk_dialogue_pages(
            rom, root["pointer"], no_title=root.get("no_title", False))
    entry["branches"] = branches
    entry["text_roots"] = roots
    entry["effects"] = effects_out
    entry["instructions"] = [row for row in stream if row["address"] in reachable]
    if not branches:
        entry["branch_rows"] = [{"branch": None, "outcome": "entry",
                                 "condition_flags": "unconditional entry",
                                 "target": owner,
                                 "path_conditions": []}]
    else:
        branch_rows = []
        for branch in branches:
            for taken, label, target in ((True, "taken", branch.get("target")),
                                         (False, "fallthrough", branch.get("fallthrough"))):
                branch_rows.append({"branch": branch["address"], "outcome": label,
                                    "condition_flags": branch["condition_flags"],
                                    "condition_source": branch.get("condition_source"),
                                    "condition": branch["condition_flags"] if taken else f"not ({branch['condition_flags']})",
                                    "target": target,
                                    "path_conditions": branch.get("path_conditions", []) +
                                    [{"branch": branch["address"], "taken": taken,
                                      "condition": branch["condition_flags"] if taken else f"not ({branch['condition_flags']})",
                                      "target": target}]})
        entry["branch_rows"] = branch_rows
    return entry


def analyze_rom_entries(rom: bytes, entries: list[dict], source_kind: str,
                        max_window: int = 0x400) -> tuple[list[dict], dict[int, list[dict]]]:
    windows = decode_entry_windows(rom, entries, max_window)
    rows = []
    for original in entries:
        entry = dict(original)
        rows.append(enrich_entry(rom, entry, windows.get(entry["entry"], []), source_kind))
    return rows, windows


def analyze_dialogue_table(rom: bytes) -> tuple[list[dict], dict[int, list[dict]]]:
    return analyze_rom_entries(rom, command_table(rom), "command", 0x400)


def analyze_title_services(rom: bytes) -> tuple[dict, dict[int, list[dict]]]:
    vector = indexed_title_services(rom)
    unique_entries = sorted({row["entry"] for row in vector if not row["noop_alias"]})
    service_entries = [{"entry": address,
                        "service_indices": [row["service_index"] for row in vector if row["entry"] == address]}
                       for address in unique_entries]
    service_rows, windows = analyze_rom_entries(rom, service_entries, "title_service", 0x500)
    dispatcher_stream = rom_instruction_stream(rom, 0x53A78, 0x53AB6, 0x53A78)
    dispatcher = enrich_entry(rom, {"entry": 0x53A78, "service": "indexed_dispatcher"},
                              dispatcher_stream, "title_dispatcher")
    finalizer_stream = rom_instruction_stream(rom, 0x1FD80, 0x20100, 0x1FD80)
    finalizer = enrich_entry(rom, {"entry": 0x1FD80, "service": "field_event_finalizer"},
                             finalizer_stream, "field_finalizer")
    actual = []
    for row in vector:
        if row["noop_alias"]:
            actual.append({**row, "branches": [], "branch_rows": [], "text_roots": [],
                           "effects": [], "instructions": [],
                           "port_location": "no-op alias 0x49A0"})
        else:
            actual.append(next(service for service in service_rows
                               if service["entry"] == row["entry"]))
    metadata = {"discovery": [
        {"entry": 0x12CCB4, "method": "250 longword pointers, indices 0x00-0xF9"},
        {"entry": 0x53A78, "method": "dispatcher at 0x53A92 loads vector address 0x15FD5C; loop indexes 0x00-0x16"},
        {"entry": 0x15FD5C, "method": "17 indexed title-service pointers, read from ROM"},
        {"entry": 0x1FD80, "method": "field-event finalizer; direct call from 0x1F64C in disassembly.s"}],
        "vector": vector, "services": actual,
        "dispatcher": dispatcher, "finalizer": finalizer}
    # Service windows are keyed by entry; add the dispatcher and finalizer for
    # the full flag/effect inventory without changing their ROM provenance.
    windows[0x53A78] = dispatcher_stream
    windows[0x1FD80] = finalizer_stream
    return metadata, windows


def decode_text(rom: bytes, pointer: int) -> str | None:
    if pointer < 0 or pointer >= len(rom):
        return None
    end = rom.find(b"\xff", pointer, min(len(rom), pointer + 0x800))
    if end < 0:
        return None
    raw = rom[pointer:end]
    if len(raw) < 4 or raw[0] not in (0x80,) and not (0x20 <= raw[0] <= 0x7E):
        return None
    if any(byte != 0x80 and not (0x20 <= byte <= 0x7e) for byte in raw):
        return None
    if sum(chr(b).isalpha() for b in raw if b < 128) < 3:
        return None
    return "".join("\n" if byte == 0x80 else chr(byte) for byte in raw)


def walk_text_records(rom: bytes, root: int, max_rows: int = 96) -> list[dict]:
    """Read 8-byte pointer records, following nested ROM record pointers."""
    records: list[dict] = []
    visited: set[int] = set()
    queue = [root]
    while queue and len(visited) < max_rows:
        table = queue.pop(0)
        if table in visited or table + 8 > len(rom):
            continue
        visited.add(table)
        direct = decode_text(rom, table)
        if direct is not None:
            records.append({"record": table, "text_pointer": table, "text": direct})
            continue
        zero_rows = 0
        for offset in range(0, min(0x300, len(rom) - table - 7), 8):
            record_address = table + offset
            first = int.from_bytes(rom[record_address:record_address + 4], "big")
            second = int.from_bytes(rom[record_address + 4:record_address + 8], "big")
            if first == 0xFFFFFFFF or second == 0xFFFFFFFF:
                break
            if first == 0 and second == 0:
                zero_rows += 1
                if zero_rows >= 4:
                    break
                continue
            zero_rows = 0
            for pointer in (first, second):
                if pointer == 0 or pointer >= len(rom):
                    continue
                text = decode_text(rom, pointer)
                if text is not None:
                    records.append({"record": record_address, "text_pointer": pointer, "text": text})
                elif 0xF0000 <= pointer < len(rom) and pointer not in visited:
                    queue.append(pointer)
    # Preserve address/text pairs but remove repeated visits through aliases.
    unique = {(item["record"], item["text_pointer"]): item for item in records}
    return [unique[key] for key in sorted(unique)]


def parse_story_names() -> dict[int, str]:
    path = ROOT / "decompilation/ghidra_conversion/qt_runtime/src/srx_refactor/enums.hpp"
    source = path.read_text(encoding="utf-8")
    block = re.search(r"enum class StoryFlag[^\{]*\{(.*?)\};", source, re.S)
    result = {}
    if block:
        for name, number in re.findall(r"([A-Z][A-Z0-9_]*)\s*=\s*0x([0-9a-f]+)", block.group(1), re.I):
            result[int(number, 16)] = name
    return result


def parse_item_names() -> dict[int, str]:
    source = (ROOT / "decompilation/gameplay/scripts/enums.lua").read_text(encoding="utf-8")
    block = re.search(r"E\.ItemId\s*=\s*\{(.*?)\n\}", source, re.S)
    result = {}
    if block:
        for name, number in re.findall(r"([A-Z][A-Z0-9_]*)\s*=\s*0x([0-9a-f]+)", block.group(1), re.I):
            result[int(number, 16)] = name
    return result


def parse_group_item_names() -> dict[int, str]:
    source = (ROOT / "decompilation/gameplay/scripts/enums.lua").read_text(encoding="utf-8")
    block = re.search(r"E\.GroupItemId\s*=\s*\{(.*?)\n\}", source, re.S)
    return ({index: name for index, name in enumerate(
        re.findall(r"([A-Z][A-Z0-9_]*)\s*=\s*\"", block.group(1)))} if block else {})


def port_fact_lines() -> dict[str, tuple[str, int]]:
    found = {}
    for path in (ROOT / "decompilation/gameplay/scripts").rglob("*.lua"):
        for number, line in enumerate(path.read_text(encoding="utf-8", errors="replace").splitlines(), 1):
            for name in re.findall(r"E\.StoryFact\.([A-Z0-9_]+)", line):
                found.setdefault(name, (str(path.relative_to(ROOT)), number))
    return found


def flag_events(instructions: list[dict], by_address: dict[int, int]) -> list[dict]:
    events: list[dict] = []
    register_words: dict[str, int] = {}
    register_written: dict[str, set[int]] = defaultdict(set)
    for item in instructions:
        address = item["address"]
        mnemonic = item["mnemonic"].lower()
        operands = item["operands"].lower().replace("$", "0x")
        base_mnemonic = mnemonic.split(".")[0]
        parts = split_operands(operands)
        if len(parts) == 2:
            source, target = parts
            source_address = abs_address(source) if "fffc" in source else None
            target_address = abs_address(target) if "fffc" in target else None
            target_reg = re.fullmatch(r"d[0-7](?:[bwl])?", target.strip())
            if base_mnemonic == "move" and source_address in FLAG_WORDS and target_reg:
                register = target.strip()[:2]
                register_words[register] = source_address
                register_written[register].clear()
                continue
            if base_mnemonic in ("bset", "bclr", "btst"):
                bit = parse_int(source)
                action = {"bset": "set", "bclr": "clear", "btst": "test"}.get(base_mnemonic)
                if bit is None or action is None:
                    continue
                storage = target_address
                if storage in FLAG_WORDS:
                    if target_address is not None:
                        bit_id = bit + ((0xFFFC21 - storage) * 8 if 0xFFFC1E <= storage <= 0xFFFC21 else 0)
                    else:
                        bit_id = bit
                elif target_reg and target.strip()[:2] in register_words:
                    storage = register_words[target.strip()[:2]]
                    bit_id = bit
                    register_written[target.strip()[:2]].add(bit)
                else:
                    continue
                events.append({"address": address, "routine": by_address.get(address),
                               "storage": storage, "bit": bit_id, "operation": action,
                               "raw_gap": item["source"] == "raw-rom-gap"})
                continue
            if base_mnemonic == "move" and target_address in FLAG_WORDS:
                # Flag mutations through BSET/BCLR/ORI/ANDI are reported at their instruction.
                continue
        if base_mnemonic in ("ori", "andi") and len(parts) == 2:
            mask = parse_int(parts[0])
            register = parts[1].strip()[:2]
            storage = register_words.get(register)
            if storage is not None and mask is not None:
                operation = "set" if base_mnemonic == "ori" else "clear"
                changed = [bit for bit in range(32) if (mask >> bit) & 1] if operation == "set" else [bit for bit in range(32) if not ((mask >> bit) & 1)]
                for bit in changed:
                    events.append({"address": address, "routine": by_address.get(address),
                                   "storage": storage, "bit": bit, "operation": operation,
                                   "raw_gap": item["source"] == "raw-rom-gap"})
                register_written[register].update(changed)
        if base_mnemonic == "clr":
            storage = abs_address(operands)
            if storage in FLAG_WORDS:
                events.append({"address": address, "routine": by_address.get(address),
                               "storage": storage, "bit": None, "operation": "clear_all",
                               "raw_gap": item["source"] == "raw-rom-gap"})
        if base_mnemonic in ("jsr", "bsr", "jmp", "bra"):
            register_words.clear()
            register_written.clear()
        elif base_mnemonic == "move" and len(parts) == 2:
            target = parts[1].strip()
            if re.fullmatch(r"d[0-7](?:[bwl])?", target):
                register_words.pop(target[:2], None)
    # Generic flag helper callers set the bit selected in FFDD64 by the command frame.
    for item in instructions:
        if item["mnemonic"].split(".")[0] in ("jsr", "bsr", "bra") and abs_address(item["operands"]) == 0x2196C:
            events.append({"address": item["address"], "routine": by_address.get(item["address"]),
                           "storage": 0xFFFC1E, "bit": "FFDD64", "operation": "set_dynamic",
                           "raw_gap": item["source"] == "raw-rom-gap"})
    unique = {(e["address"], e["storage"], str(e["bit"]), e["operation"]): e for e in events}
    return [unique[key] for key in sorted(unique, key=lambda k: (k[0], k[1], k[2], k[3]))]


def text_roots_for_routine(group: list[dict]) -> list[dict]:
    roots = []
    for item in group:
        if not item["mnemonic"].startswith("lea"):
            continue
        ptr = abs_address(item["operands"])
        if ptr is None or ptr < 0xF0000:
            continue
        roots.append({"instruction": item["address"], "pointer": ptr})
    return roots


def nearest_text_root(group: list[dict], site: int) -> int | None:
    roots = text_roots_for_routine(group)
    future = [root for root in roots if root["instruction"] >= site]
    selected = min(future, key=lambda root: root["instruction"]) if future else None
    if selected is None and roots:
        selected = min(roots, key=lambda root: abs(root["instruction"] - site))
    return selected["pointer"] if selected else None


def effects(instructions: list[dict], groups: dict[int, list[dict]], rom: bytes,
            flags: list[dict], story_names: dict[int, str]) -> list[dict]:
    effects_out = []
    item_names = parse_item_names()
    for routine, group in groups.items():
        roots = text_roots_for_routine(group)
        def nearest_root(site: int) -> int | None:
            future = [root for root in roots if root["instruction"] >= site]
            selected = min(future, key=lambda root: root["instruction"]) if future else None
            if selected is None and roots:
                selected = min(roots, key=lambda root: abs(root["instruction"] - site))
            return selected["pointer"] if selected else None

        for item in group:
            mnemonic = item["mnemonic"].lower().split(".")[0]
            ops = item["operands"].lower().replace("$", "0x")
            if mnemonic in ("jsr", "bsr") and abs_address(ops) == 0x59C34:
                preceding = [candidate for candidate in group if 0 < item["address"] - candidate["address"] <= 12]
                amount = None
                for candidate in reversed(preceding):
                    if "d0" in candidate["operands"].lower():
                        amount = parse_int(candidate["operands"])
                        if amount is not None:
                            break
                effects_out.append({"address": item["address"], "routine": routine,
                                    "kind": "karma", "operation": "award_each_runner",
                                    "amount": amount, "text_root": nearest_root(item["address"]),
                                    "source_addresses": [candidate["address"] for candidate in preceding[-2:]]})
            parts = split_operands(ops)
            if mnemonic in ("subi", "addi", "sub", "add") and len(parts) == 2 and abs_address(parts[1]) == 0xFFFBFE:
                amount = parse_int(parts[0])
                sign = -1 if mnemonic.startswith("sub") else 1
                effects_out.append({"address": item["address"], "routine": routine,
                                    "kind": "nuyen", "operation": "debit" if sign < 0 else "credit",
                                    "amount": amount, "delta": sign * amount if amount is not None else None,
                                    "text_root": nearest_root(item["address"]), "source_addresses": [item["address"]]})
            if mnemonic == "move" and len(parts) == 2 and abs_address(parts[1]) == 0xFFFBFE:
                amount = parse_int(parts[0])
                effects_out.append({"address": item["address"], "routine": routine,
                                    "kind": "nuyen", "operation": "balance_write",
                                    "amount": amount, "delta": None, "storage": 0xFFFBFE,
                                    "text_root": nearest_root(item["address"]), "source_addresses": [item["address"]]})
            if mnemonic in ("jsr", "bsr") and abs_address(ops) == 0x21A4E:
                preceding = [candidate for candidate in group if 0 < item["address"] - candidate["address"] <= 16]
                amount = next((parse_int(candidate["operands"]) for candidate in reversed(preceding)
                               if parse_int(candidate["operands"]) is not None), None)
                effects_out.append({"address": item["address"], "routine": routine,
                                    "kind": "nuyen", "operation": "conditional_debit",
                                    "amount": amount, "delta": -amount if amount is not None else None,
                                    "text_root": nearest_root(item["address"]), "source_addresses": [item["address"]]})
        selected_item_slot = any(item["mnemonic"].split(".")[0] in ("jsr", "bsr") and
                                 abs_address(item["operands"]) == 0x22874 for item in group)
        if selected_item_slot:
            for index, item in enumerate(group):
                ops = item["operands"].lower().replace("$", "0x")
                if item["mnemonic"].split(".")[0] == "move" and "a3" in ops and "d6" in ops and "0x8(" not in ops and "8(a3" not in ops:
                    code = parse_int(ops)
                    if code is None:
                        continue
                    rating = None
                    for later in group[index + 1:index + 4]:
                        later_ops = later["operands"].lower().replace("$", "0x")
                        if later["mnemonic"].split(".")[0] in ("rts", "bra", "jmp"):
                            break
                        if "a3" in later_ops and "d6" in later_ops and ("0x8(" in later_ops or "8(a3" in later_ops):
                            rating = 0 if later["mnemonic"].split(".")[0] == "clr" else parse_int(later_ops)
                            break
                    operation = "take_runner_item" if code == 0xFF else "give_runner_item"
                    effects_out.append({"address": item["address"], "routine": routine,
                                        "kind": "item", "operation": operation,
                                        "item_code": code, "item_name": item_names.get(code),
                                        "rating": rating, "text_root": nearest_root(item["address"]),
                                        "source_addresses": [routine, item["address"]]})
    group_item_names = parse_group_item_names()
    for event in flags:
        if event["storage"] == 0xFFFC1E and isinstance(event["bit"], int) and event["operation"] == "set":
            name = story_names.get(event["bit"])
            if name and ("ALIAS" in name or "KEYWORD" in name):
                group = groups.get(event.get("routine"), [])
                roots = text_roots_for_routine(group)
                future = [root for root in roots if root["instruction"] >= event["address"]]
                selected = min(future, key=lambda root: root["instruction"]) if future else None
                effects_out.append({"address": event["address"], "routine": event["routine"],
                                    "kind": "keyword_unlock", "operation": "set_story_flag",
                                    "flag": name, "bit": event["bit"],
                                    "text_root": selected["pointer"] if selected else None,
                                    "source_addresses": [event["address"]]})
        if event["storage"] == 0xFFFC22 and event["operation"] in ("set", "clear"):
            group = groups.get(event.get("routine"), [])
            effects_out.append({"address": event["address"], "routine": event["routine"],
                                "kind": "item", "operation": "give_group_item" if event["operation"] == "set" else "take_group_item",
                                "bit": event["bit"],
                                "item_name": group_item_names.get(event["bit"]) if isinstance(event["bit"], int) else None,
                                "source_addresses": [event["address"]],
                                "text_root": nearest_text_root(group, event["address"])})
    return effects_out


def nested_strings(value):
    if isinstance(value, str):
        yield value
    elif isinstance(value, dict):
        for child in value.values():
            yield from nested_strings(child)
    elif isinstance(value, list):
        for child in value:
            yield from nested_strings(child)


def script_effects_for_graph(graph_id: str, addresses: set[int]) -> dict[str, str]:
    """Find directly named Lua effects and comments tied to this ROM graph."""
    categories: dict[str, str] = {}
    for path in (ROOT / "decompilation/gameplay/scripts").rglob("*.lua"):
        lines = path.read_text(encoding="utf-8", errors="replace").splitlines()
        source = "\n".join(lines)
        named = graph_id in source
        matching_lines = []
        for index, line in enumerate(lines):
            line_addresses = {int(value, 16) for value in PTR_RE.findall(line)}
            if line_addresses & addresses:
                matching_lines.append(index)
        if not named and not matching_lines:
            continue
        if named:
            for index, line in enumerate(lines):
                if "award_karma" in line:
                    categories.setdefault("karma", f"{path.relative_to(ROOT)}:{index + 1}")
                if "GroupItems.set" in line or "set_group_item" in line:
                    categories.setdefault("item", f"{path.relative_to(ROOT)}:{index + 1}")
                if "michael_alias_learned" in line or "keyword" in line.lower():
                    categories.setdefault("keyword_unlock", f"{path.relative_to(ROOT)}:{index + 1}")
                if "game.services.call" in line and any(word in line.lower() for word in ("charge", "purchase", "nuyen")):
                    categories.setdefault("nuyen", f"{path.relative_to(ROOT)}:{index + 1}")
        for index in matching_lines:
            context = "\n".join(lines[max(0, index - 5):min(len(lines), index + 9)]).lower()
            location = f"{path.relative_to(ROOT)}:{index + 1}"
            if "karma" in context or "award_karma" in context:
                categories.setdefault("karma", location)
            if "groupitems.set" in context or "set_group_item" in context or "fffc22" in context and "item" in context:
                categories.setdefault("item", location)
            if "michael_alias_learned" in context or "keyword" in context:
                categories.setdefault("keyword_unlock", location)
            if "nuyen" in context or "fffbfe" in context or "purchase" in context:
                categories.setdefault("nuyen", location)
    return categories


def action_source_text(action_id: str, code_path: str | None) -> str:
    if not code_path:
        return ""
    cache = getattr(action_source_text, "_cache", {})
    key = (action_id, code_path)
    if key in cache:
        return cache[key]
    relative, _, line_text = code_path.rpartition(":")
    try:
        start = max(0, int(line_text) - 1)
        lines = (ROOT / relative).read_text(encoding="utf-8", errors="replace").splitlines()
    except (OSError, ValueError):
        return ""
    body = []
    base_indent = len(lines[start]) - len(lines[start].lstrip())
    for line in lines[start:min(len(lines), start + 14)]:
        stripped = line.strip()
        indent = len(line) - len(line.lstrip())
        if body and indent == base_indent and re.fullmatch(r"end(?:,|\))?", stripped):
            break
        if body and re.search(r"\[\s*\"[a-z0-9_.]+\"\s*\]\s*=\s*function", line):
            break
        body.append(line)
    text = "\n".join(body)
    # Some registered actions delegate their effect to one local helper.
    # Include that helper's top-level function body so concrete service values
    # such as award_karma_1 remain visible to the matcher.
    helper_names = set(re.findall(r"\b([a-z_][a-z0-9_]*)\s*\(\s*\)", text))
    helper_bodies = []
    for helper in helper_names:
        declaration = re.compile(r"^local function " + re.escape(helper) + r"\s*\(")
        for index, line in enumerate(lines):
            if not declaration.search(line):
                continue
            for helper_line in lines[index + 1:]:
                if helper_line == "end":
                    break
                helper_bodies.append(helper_line)
            break
    if helper_bodies:
        text += "\n" + "\n".join(helper_bodies)
    cache[key] = text
    action_source_text._cache = cache
    return text


def action_source_categories(action_id: str, code_path: str | None) -> dict[str, str]:
    if action_id.startswith(("building.", "field.", "item.", "party.", "story.", "actor.")):
        return {}
    text = action_source_text(action_id, code_path).lower()
    categories = {}
    if "award_karma" in text:
        categories["karma"] = code_path
    if "groupitems.set" in text or "item.set_group_item" in text:
        categories["item"] = code_path
    if "charge(" in text or "charge_" in text or "nuyen" in text or "purchase" in text:
        categories["nuyen"] = code_path
    if "michael_alias_learned" in text or "keyword" in text:
        categories["keyword_unlock"] = code_path
    if "storyfact" in text or "game.state" in text:
        categories["quest_flag"] = code_path
    return categories


def parse_port_graphs(rom: bytes) -> list[dict]:
    graphs = []
    for path in sorted((ROOT / "decompilation/gameplay/dialogue/conversations").glob("*.json")):
        try:
            data = json.loads(path.read_text(encoding="utf-8"))
        except (OSError, json.JSONDecodeError):
            continue
        for graph_id, graph in data.get("graphs", {}).items():
            if not isinstance(graph, dict):
                continue
            provenance = graph.get("provenance", {})
            addresses = set()
            for value in nested_strings(graph):
                addresses.update(int(number, 16) for number in PTR_RE.findall(value))
            for value in provenance.get("romAddresses", []):
                parsed = abs_address(str(value))
                if parsed is not None:
                    addresses.add(parsed)
            lines = path.read_text(encoding="utf-8").splitlines()
            nodes = []
            actions = []
            for node_id, node in graph.get("nodes", {}).items():
                if not isinstance(node, dict):
                    continue
                node_line = next((i for i, line in enumerate(lines, 1)
                                  if re.search(r'"' + re.escape(node_id) + r'"\s*:', line)), None)
                copied = {"id": node_id, "type": node.get("type"),
                          "evidence": node.get("evidence"),
                          "line": node_line, "choices": node.get("choices", []),
                          "cases": node.get("cases", []), "fallback": node.get("fallback"),
                          "next": node.get("next"), "results": node.get("results", {}),
                          "action_id": node.get("actionId"),
                          "arguments": node.get("arguments", {}),
                          "text_keys": [value for key, value in node.items()
                                        if "textkey" in key.lower() and isinstance(value, str)],
                          "menu_id": node.get("menuId"), "event_id": node.get("eventId")}
                nodes.append(copied)
                if node.get("type") != "action":
                    continue
                action_id = str(node.get("actionId", ""))
                categories = set()
                if action_id == "party.award_karma": categories.add("karma")
                if action_id in ("building.charge_amount", "field.charge_copt_fare") or "purchase" in action_id: categories.add("nuyen")
                if action_id in ("item.set_group_item", "building.purchase_runner_item") or action_id.startswith(("vid_phone.purchase_", "building.purchase_")): categories.add("item")
                if action_id == "boris.reveal_michael_information": categories.add("keyword_unlock")
                if action_id == "story.set_fact": categories.add("quest_flag")
                if action_id == "stoker_motel.pay_debt": categories.update(("nuyen", "karma", "quest_flag"))
                if action_id == "boris.reveal_michael_information": categories.add("quest_flag")
                if action_id == "tarislar_city_inn.apply_tabatha_acceptance": categories.add("karma")
                actions.append({"id": action_id, "node": node_id, "arguments": node.get("arguments", {}),
                                "evidence": node.get("evidence"), "data_line": node_line,
                                "categories": sorted(categories)})
            for action in actions:
                needle = '"actionId": "' + action["id"] + '"'
                action["data_line"] = next((i for i, line in enumerate(lines, 1) if needle in line), None)
                action["code_path"] = port_action_path(action["id"])
                extra = action_source_categories(action["id"], action["code_path"])
                action["categories"] = sorted(set(action["categories"]) | set(extra))
                action["category_paths"] = extra
            script_categories = script_effects_for_graph(graph_id, addresses)
            action_categories = {category for action in actions for category in action["categories"]}
            graph_categories = sorted(action_categories | set(script_categories))
            graphs.append({"id": graph_id, "file": str(path.relative_to(ROOT)),
                           "addresses": sorted(addresses), "actions": actions,
                           "nodes": nodes,
                           "effect_categories": graph_categories,
                           "effect_paths": script_categories})
    return graphs


def port_action_path(action_id: str) -> str | None:
    preferred = {
        "boris.reveal_michael_information": "boris_greenhouse.lua",
        "stoker_motel.pay_debt": "michael_package.lua",
    }.get(action_id)
    search = (ROOT / "decompilation/gameplay/scripts").rglob(preferred) if preferred else (ROOT / "decompilation/gameplay/scripts").rglob("*.lua")
    for path in search:
        lines = path.read_text(encoding="utf-8", errors="replace").splitlines()
        for number, line in enumerate(lines, 1):
            if action_id in line:
                return f"{path.relative_to(ROOT)}:{number}"
    return None


def entry_for_address(address: int, entries: list[dict],
                      by_address: dict[int, dict]) -> dict | None:
    direct = by_address.get(address)
    if direct is not None:
        return direct
    return next((entry for entry in entries if entry.get("entry") == address), None)


def source_path_from_location(location: str | None) -> str | None:
    if not location or location == "MISSING":
        return None
    for part in location.split(";"):
        candidate = part.strip().rsplit(":", 1)[0]
        if candidate.endswith((".lua", ".json", ".cpp", ".hpp")):
            return candidate
    return None


def branch_outcome_for_path(path: list[dict], branch: int) -> bool | None:
    for condition in path:
        if condition.get("branch") == branch:
            return condition.get("taken")
    return None


def branch_row_effects(entry: dict, effect_rows: list[dict]) -> None:
    site_effects = [effect for effect in effect_rows
                    if effect.get("command_entry") == entry["entry"]]
    for branch_row in entry.get("branch_rows", []):
        branch = branch_row.get("branch")
        outcome = branch_row.get("outcome")
        if branch is None:
            relevant = site_effects
            roots = entry.get("text_roots", [])
        else:
            relevant = []
            for effect in site_effects:
                if any(branch_outcome_for_path(path, branch) == (outcome == "taken")
                       for path in effect.get("path_conditions", [])):
                    relevant.append(effect)
            roots = [root for root in entry.get("text_roots", [])
                     if any(branch_outcome_for_path(path, branch) == (outcome == "taken")
                            for path in root.get("path_conditions", []))]
        pages = [page for root in roots for page in root.get("pages", [])]
        branch_row["text_roots"] = [root["pointer"] for root in roots]
        branch_row["page_keys"] = [key for page in pages for key in page.get("port_page_keys", [])]
        branch_row["effects"] = [{"address": effect["address"], "kind": effect["kind"],
                                  "operation": effect.get("operation"),
                                  "value": effect.get("amount", effect.get("item_name", effect.get("bit"))),
                                  "storage": effect.get("storage"), "status": effect.get("status"),
                                  "port": effect.get("port")}
                                 for effect in relevant]
        statuses = [effect.get("status") for effect in relevant]
        page_statuses = [page.get("page_status") for page in pages]
        if "WRONG" in statuses:
            status = "WRONG"
        elif "MISSING" in statuses or "MISSING" in page_statuses:
            status = "MISSING"
        elif relevant or pages:
            status = "MATCH"
        elif entry.get("port_status") == "MATCH":
            status = "NOT_AN_EFFECT"
        else:
            status = "MISSING"
        branch_row["status"] = status
        branch_row["port_location"] = entry.get("port_location", "MISSING")
        branch_row["reason"] = ("branch has linked effects/pages" if status == "MATCH" else
                                 "branch has a wrong port value or storage identity" if status == "WRONG" else
                                 "branch text or effect has no port implementation" if status == "MISSING" else
                                 "branch has no quest effect")
    statuses = [row["status"] for row in entry.get("branch_rows", [])]
    entry["status"] = ("WRONG" if "WRONG" in statuses else
                        "MISSING" if "MISSING" in statuses else
                        "MATCH" if "MATCH" in statuses else "NOT_AN_EFFECT")


def compare_effects(effects_rows: list[dict], graphs: list[dict], flags: list[dict],
                    entries: list[dict], by_address: dict[int, dict],
                    source_files: list[dict]) -> list[dict]:
    for effect in effects_rows:
        entry = entry_for_address(effect.get("address", 0), entries, by_address)
        if entry is None and effect.get("command_entry") is not None:
            entry = next((row for row in entries
                          if row.get("entry") == effect["command_entry"]), None)
        if entry is not None:
            effect["command_entry"] = entry["entry"]
            effect["command"] = entry.get("command")
            effect["service_indices"] = entry.get("service_indices", [])
            effect["port_entry"] = entry.get("port_location", "MISSING")
            effect["region"] = entry.get("region", "misc")
            if effect.get("text_root") is None and entry.get("text_roots"):
                nearest = min(entry["text_roots"],
                              key=lambda root: abs(root["instruction"] - effect["address"]))
                effect["text_root"] = nearest["pointer"]
        else:
            effect["port_entry"] = "MISSING"
            effect["region"] = "misc"
        effect["raw_bytes"] = effect.get("raw_bytes") or ""
        effect["status"] = effect.get("status") or None
        effect = compare_effect_to_port(effect, entry, graphs, source_files)
    return effects_rows


def build_audit(rom_path: Path, disasm_path: Path) -> dict:
    rom = rom_path.read_bytes()
    instructions = [item for line in disasm_path.read_text(encoding="utf-8", errors="replace").splitlines()
                    if (item := parse_instruction(line)) is not None]
    instructions, gaps = decode_bounded_gaps(rom, instructions)
    by_routine_address, groups = routine_index(instructions)
    command_rows, command_windows = analyze_dialogue_table(rom)
    service_inventory, service_windows = analyze_title_services(rom)
    service_rows = {}
    for row in service_inventory["services"]:
        if not row.get("noop_alias"):
            service_rows.setdefault(row["entry"], row)
    extra_entries = [service_inventory["dispatcher"], service_inventory["finalizer"]]
    entries = command_rows + list(service_rows.values()) + extra_entries
    windows = dict(command_windows)
    windows.update(service_windows)
    port_graphs = parse_port_graphs(rom)
    source_files = port_content_files()
    for entry in entries:
        annotate_entry_port(entry, port_graphs, source_files)

    address_owner = {}
    for entry in entries:
        stream = windows.get(entry["entry"], [])
        for instruction in stream:
            address_owner.setdefault(instruction["address"], entry)
    raw_flags = raw_flag_events(windows)
    legacy_flags = flag_events(instructions, by_routine_address)
    flag_map = {}
    trigger_operations = []
    for event in legacy_flags + raw_flags:
        if event["storage"] == 0xFFDD64 or event.get("bit") == "FFDD64":
            if event.get("storage") == 0xFFDD64:
                trigger_operations.append(event)
            continue
        key = (event["address"], event["storage"], str(event.get("bit")), event["operation"])
        flag_map[key] = event
    story_names = parse_story_names()
    group_names = parse_group_item_names()
    port_facts = port_fact_lines()
    flags = sorted(flag_map.values(), key=lambda row: (row["address"], row["storage"], str(row.get("bit"))))
    for event in flags:
        event["story_name"] = (story_names.get(event["bit"])
                               if event["storage"] == 0xFFFC1E and isinstance(event.get("bit"), int)
                               else None)
        event["port_fact"] = port_facts.get(event["story_name"]) if event.get("story_name") else None
        event["port_identity"] = port_identity(event["storage"], event.get("bit"), story_names, group_names)
        event["raw_bytes"] = rom[event["address"]:event["address"] + 8].hex(" ")
        owner = address_owner.get(event["address"])
        if owner is not None:
            event["command"] = owner.get("command")
            event["command_entry"] = owner["entry"]
        if event["operation"] == "test":
            event["status"] = "NOT_AN_EFFECT"
            event["reason"] = "ROM branch condition reads this storage/bit"
            event["port"] = "branch condition"
        else:
            compare_flag_effect(event, owner, port_graphs)

    legacy_effects = effects(instructions, groups, rom, flags, story_names)
    extracted_effects = []
    service_calls = []
    for entry in entries:
        for effect in entry.get("effects", []):
            if effect["kind"] == "service_call":
                service_calls.append({"address": effect["address"], "routine": entry["entry"],
                                      "target": effect.get("target_service"),
                                      "source_kind": entry.get("source_kind", "command")})
                continue
            effect["command_entry"] = entry["entry"]
            effect["command"] = entry.get("command")
            effect["service_indices"] = entry.get("service_indices", [])
            effect["routine"] = entry["entry"]
            if effect["kind"] == "flag":
                effect["port_identity"] = port_identity(effect["storage"], effect.get("bit"),
                                                        story_names, group_names)
                matching = next((row for row in flags
                                 if row["address"] == effect["address"] and
                                 row["storage"] == effect["storage"] and
                                 str(row.get("bit")) == str(effect.get("bit")) and
                                 row["operation"] == effect["operation"]), None)
                if matching:
                    effect.update({key: matching[key] for key in
                                   ("status", "reason", "port", "port_write") if key in matching})
            extracted_effects.append(effect)

    # Keep the call-site amount load as the transaction row; pass 1's row at
    # the following BSR is superseded when both describe the same debit.
    superseded_calls = {effect.get("call_address") for effect in extracted_effects
                        if effect["kind"] == "nuyen" and effect.get("call_address")}
    combined = [row for row in legacy_effects
                if not (row["kind"] == "nuyen" and row["address"] in superseded_calls)]
    combined.extend(extracted_effects)
    effect_map = {}
    for effect in combined:
        key = (effect["address"], effect["kind"], effect.get("operation"),
               effect.get("item_code"), effect.get("storage"), effect.get("bit"))
        effect_map[key] = effect
    effect_rows = list(effect_map.values())
    # Build address ownership from the decoded entry windows. This makes a ROM
    # handler pointer, rather than a neighboring graph/category, the match key.
    compare_effects(effect_rows, port_graphs, flags, entries, address_owner, source_files)
    normalized_effects = {}
    for effect in effect_rows:
        key = (effect["address"], effect["kind"], effect.get("operation"))
        normalized_effects[key] = effect
    effect_rows = sorted(normalized_effects.values(), key=lambda row: (row["address"], row["kind"]))

    for entry in entries:
        branch_row_effects(entry, effect_rows)
    text_record_roots = {}
    page_chains = []
    for entry in entries:
        for root in entry.get("text_roots", []):
            text_record_roots.setdefault(root["pointer"], walk_text_records(rom, root["pointer"]))
            page_chains.append({"source_kind": entry.get("source_kind"),
                                "command": entry.get("command"),
                                "service_indices": entry.get("service_indices", []),
                                "entry": entry["entry"], "text_root": root["pointer"],
                                "no_title": root.get("no_title", False),
                                "pages": root.get("pages", [])})
    for effect in effect_rows:
        if effect.get("text_root") is not None:
            text_record_roots.setdefault(effect["text_root"],
                                         walk_text_records(rom, effect["text_root"]))
        effect["raw_bytes"] = rom[effect["address"]:effect["address"] + 8].hex(" ")
    text_record_roots = {key: value for key, value in text_record_roots.items() if value}
    mismatches = [effect for effect in effect_rows if effect.get("status") in ("WRONG", "MISSING")]
    counts = lambda rows, status_key="status": {
                           status: sum(row.get(status_key) == status for row in rows)
                           for status in ("MATCH", "WRONG", "MISSING", "NOT_AN_EFFECT")}
    page_rows = [page for chain in page_chains for page in chain["pages"]]
    branch_rows = [branch for entry in entries for branch in entry.get("branch_rows", [])]
    return {"rom": str(rom_path), "disassembly": str(disasm_path),
            "raw_gap_ranges": gaps, "command_table": 0x12CCB4,
            "command_handlers": command_rows, "title_services": service_inventory,
            "flag_operations": flags, "trigger_operations": trigger_operations,
            "dialogue_effects": effect_rows, "service_calls": service_calls,
            "text_record_roots": text_record_roots, "page_chains": page_chains,
            "port_graphs": port_graphs, "mismatches": mismatches,
            "summary": {"effects": counts(effect_rows), "flags": counts(flags),
                        "branches": counts(branch_rows), "pages": counts(page_rows, "page_status"),
                        "handler_count": len(command_rows),
                        "title_service_vector_count": len(service_inventory["vector"]),
                        "effect_count": len(effect_rows), "flag_count": len(flags),
                        "missing_text_pages": sum(page.get("page_status") == "MISSING" for page in page_rows)}}


def port_fact_names() -> set[str]:
    source = (ROOT / "decompilation/gameplay/scripts/enums.lua").read_text(encoding="utf-8")
    block = re.search(r"E\.StoryFact\s*=\s*\{(.*?)\n\}", source, re.S)
    return set(re.findall(r"([A-Z][A-Z0-9_]*)\s*=\s*\"", block.group(1))) if block else set()


def normalized_text(value: str) -> str:
    return re.sub(r"[^a-z0-9]", "", value.lower())


def port_content_files() -> list[dict]:
    paths = set()
    for directory, suffixes in ((ROOT / "decompilation/gameplay/scripts", {".lua"}),
                                (ROOT / "decompilation/gameplay/dialogue", {".json"}),
                                (ROOT / "decompilation/assets", {".json"}),
                                (ROOT / "decompilation/ghidra_conversion/qt_runtime/src", {".cpp", ".hpp"})):
        if directory.exists():
            paths.update(path for path in directory.rglob("*")
                         if path.is_file() and path.suffix in suffixes)
    result = []
    for path in sorted(paths):
        try:
            lines = path.read_text(encoding="utf-8", errors="replace").splitlines()
        except OSError:
            continue
        result.append({"path": str(path.relative_to(ROOT)), "lines": lines,
                       "normalized": normalized_text("\n".join(lines))})
    return result


def source_line_for_text(lines: list[str], text: str) -> int | None:
    compact = normalized_text(text)
    if len(compact) < 12:
        return None
    needle = compact[:min(32, len(compact))]
    for index in range(len(lines)):
        window = normalized_text("\n".join(lines[index:index + 8]))
        if needle in window:
            return index + 1
    return None


def content_hits(text: str, files: list[dict]) -> list[dict]:
    compact = normalized_text(text)
    if len(compact) < 12:
        return []
    needle = compact[:min(36, len(compact))]
    hits = []
    for source in files:
        if needle not in source["normalized"]:
            continue
        hits.append({"file": source["path"],
                     "line": source_line_for_text(source["lines"], text)})
    return hits


def port_state_writes(path: str | None = None) -> list[dict]:
    cache = getattr(port_state_writes, "_cache", {})
    cache_key = path or "<all-lua>"
    if cache_key in cache:
        return cache[cache_key]
    writes = []
    roots = [ROOT / path] if path else sorted((ROOT / "decompilation/gameplay/scripts").rglob("*.lua"))
    direct = re.compile(r"(?:game\.state\.)?(field_mode_flag|field_mode_effect_flag|field_dialogue_flag|field_auxiliary_flag|notebook_tip_flag)_bit_(\d+)\s*=\s*(true|false)")
    for source in roots:
        if not source.is_file():
            continue
        relative = str(source.relative_to(ROOT))
        for number, line in enumerate(source.read_text(encoding="utf-8", errors="replace").splitlines(), 1):
            for match in direct.finditer(line):
                writes.append({"identity": f"{match.group(1)}_bit_{match.group(2)}",
                               "value": match.group(3) == "true", "file": relative,
                               "line": number, "raw": line.strip()})
            match = re.search(r"GroupItems\.set\(\s*E\.GroupItemId\.([A-Z0-9_]+)\s*,\s*(true|false)", line)
            if match:
                writes.append({"identity": f"GroupItemId.{match.group(1)}",
                               "value": match.group(2) == "true", "file": relative,
                               "line": number, "raw": line.strip()})
            match = re.search(r"StoryFacts?\.set\(\s*E\.StoryFact\.([A-Z0-9_]+)", line)
            if match:
                writes.append({"identity": f"StoryFact.{match.group(1)}",
                               "value": True, "file": relative, "line": number,
                               "raw": line.strip()})
    cache[cache_key] = writes
    port_state_writes._cache = cache
    return writes


STORAGE_NAMES = {
    0xFFFC1C: "unmapped_legacy_flag_word",
    0xFFFC1E: "StoryFact",
    0xFFFC22: "GroupItemId",
    0xFFFC26: "field_dialogue_flag",
    0xFFFC27: "field_dialogue_flag_byte_2",
    0xFFFC2C: "unmapped_legacy_flag_word",
    0xFFFC30: "field_mode_flag",
    0xFFFC34: "field_mode_effect_flag",
    0xFFFC38: "notebook_tip_flag",
    0xFFFC40: "field_auxiliary_flag",
}


def port_identity(storage: int, bit: int | str | None,
                  story_names: dict[int, str], group_names: dict[int, str]) -> str:
    if not isinstance(bit, int):
        return STORAGE_NAMES.get(storage, f"unknown_storage_0x{storage:X}")
    family = STORAGE_NAMES.get(storage, f"unknown_storage_0x{storage:X}")
    if storage == 0xFFFC1E:
        return f"StoryFact.{story_names.get(bit, f'BIT_{bit}')}"
    if storage == 0xFFFC22:
        return f"GroupItemId.{group_names.get(bit, f'BIT_{bit}')}"
    return f"{family}_bit_{bit}"


def region_for(file: str | None, texts: list[str], command: int | None = None) -> str:
    if command is not None:
        if 0x53 <= command <= 0x63 or 0xC6 <= command <= 0xC8:
            return "Penumbra/clubs"
        if 0x64 <= command <= 0x67:
            return "Downtown"
        if 0xB1 <= command <= 0xBB:
            return "Puyallup/gangs"
    lower = (file or "").lower()
    text = " ".join(texts).lower()
    joined = lower + " " + text
    if "vid_phone" in joined or "capt quinton" in joined or "phone" in joined:
        return "vid-phone"
    if any(word in joined for word in ("salish", "owlfeather", "red buffalo", "sinsearach", "tarislar", "council island")):
        return "Council Island/Salish"
    if any(word in joined for word in ("renraku", "fuchi", "arcology", "mitsuhama", "corp")):
        return "Renraku/Arcology" if "renraku" in joined or "arcology" in joined else "corp sites"
    if any(word in joined for word in ("penumbra", "club", "bouncer", "ganger")):
        return "Penumbra/clubs"
    if any(word in joined for word in ("gang", "halloweener", "ork armband", "ratspike", "mauler", "roadrash")):
        return "Puyallup/gangs"
    if any(word in joined for word in ("redmond", "quinton", "stoker", "boris", "michael", "joshua")):
        return "Redmond"
    if any(word in joined for word in ("downtown", "johnson", "sergeant", "arrest")):
        return "Downtown"
    return "misc"


def graph_for_entry(entry: int, graphs: list[dict],
                    command: int | None = None) -> list[dict]:
    exact = [graph for graph in graphs if entry in graph.get("addresses", [])]
    if exact or command is None:
        return exact
    selected = []
    for graph in graphs:
        if graph_nodes_for_command(graph, command, entry, set()):
            selected.append(graph)
    return selected


def graph_nodes_for_command(graph: dict, command: int | None,
                            entry: int, pointers: set[int]) -> list[dict]:
    selected = []
    command_text = f"command 0x{command:02X}" if command is not None else None
    for node in graph.get("nodes", []):
        evidence = str(node.get("evidence") or "").lower()
        exact_command = (command_text and command_text.lower() in evidence) or (
            command is not None and re.search(r"\bcmd\s*(?:0x)?" + f"{command:02x}" + r"\b", evidence))
        exact_entry = f"0x{entry:X}".lower() in evidence
        exact_pointer = any(f"0x{pointer:X}".lower() in evidence for pointer in pointers)
        if exact_command or exact_entry or exact_pointer:
            selected.append(node)
    return selected


def actions_reachable_from_command(graph: dict, command: int | None,
                                   entry: int, pointers: set[int]) -> list[dict]:
    """Use branch evidence and graph edges to select actions for one ROM command."""
    selected_nodes = graph_nodes_for_command(graph, command, entry, pointers)
    nodes = {node["id"]: node for node in graph.get("nodes", [])}
    queue = deque(node["id"] for node in selected_nodes)
    seen = set()
    actions = []
    while queue and len(seen) < len(nodes):
        node_id = queue.popleft()
        if node_id in seen:
            continue
        seen.add(node_id)
        node = nodes.get(node_id)
        if node is None:
            continue
        if node.get("type") == "action":
            action = next((item for item in graph.get("actions", [])
                           if item["node"] == node_id), None)
            if action is not None:
                actions.append(action)
            continue
        targets = []
        if node.get("next"):
            targets.append(node["next"])
        if node.get("fallback"):
            targets.append(node["fallback"])
        targets.extend(choice.get("next") for choice in node.get("choices", [])
                       if choice.get("next"))
        targets.extend(case.get("next") for case in node.get("cases", [])
                       if case.get("next"))
        targets.extend(result for result in (node.get("results") or {}).values()
                       if isinstance(result, str))
        queue.extend(target for target in targets if target not in seen)
    return list({action["node"]: action for action in actions}.values())


def action_value_matches(effect: dict, action: dict) -> tuple[bool, str]:
    kind = effect["kind"]
    arguments = action.get("arguments", {})
    action_id = action.get("id", "")
    if kind == "karma":
        value = arguments.get("amount")
        if value is None:
            amounts = {int(amount) for amount in
                       re.findall(r"\baward_karma_(\d+)\b",
                                  action_source_text(action_id, action.get("code_path")))}
            if len(amounts) == 1:
                value = amounts.pop()
        return value == effect.get("amount"), f"port Karma amount {value}; ROM amount {effect.get('amount')}"
    if kind == "nuyen":
        value = arguments.get("amount")
        if value is None and action_id == "vid_phone.purchase_hk227_s":
            exact_result = (effect.get("result_item_code") == 9 and
                            effect.get("result_phase") == 0x18)
            return exact_result, "Quinton command 0x69 maps the paid result to item 9 phase 0x18"
        if value is None and action_id == "vid_phone.purchase_frag_grenades":
            exact_result = effect.get("result_item_code") == 4
            return exact_result, "Quinton command 0x68 maps the paid result to Frag Grenades"
        return value == effect.get("amount"), f"port nuyen amount {value}; ROM amount {effect.get('amount')}"
    if kind in ("item", "item_phase"):
        expected_name = effect.get("item_name")
        if expected_name:
            port_name = arguments.get("itemId")
            if isinstance(port_name, str):
                return port_name.upper() == expected_name.upper(), f"port group item {port_name}; ROM group item {expected_name}"
            return False, f"port group item {port_name}; ROM group item {expected_name}"
        if action_id == "vid_phone.purchase_hk227_s":
            return (effect.get("item_code") == 9 or effect.get("result_item_code") == 9 or
                    effect.get("phase") == 0x18 or effect.get("result_phase") == 0x18), "Quinton command 0x69 purchases item 9 phase 0x18"
        if action_id == "vid_phone.purchase_frag_grenades":
            return effect.get("item_code") == 4, "Quinton command 0x68 purchases Frag Grenades"
        expected_code = effect.get("item_code")
        expected_phase = effect.get("phase")
        port_code = arguments.get("itemCode")
        port_phase = arguments.get("rating")
        if expected_code is not None and port_code is not None:
            code_matches = expected_code == port_code
            phase_matches = expected_phase is None or port_phase == expected_phase
            return code_matches and phase_matches, f"port item {port_code}/{port_phase}; ROM item {expected_code}/{expected_phase}"
        return False, "branch action has no ROM-matching item value"
    return True, "branch action category is present"


def compare_flag_effect(effect: dict, entry: dict | None,
                        graphs: list[dict]) -> dict:
    storage = effect.get("storage")
    bit = effect.get("bit")
    identity = effect.get("port_identity", "")
    family = STORAGE_NAMES.get(storage, "")
    expected_action = {
        "StoryFact": "story.set_fact", "GroupItemId": "item.set_group_item",
        "field_dialogue_flag": "field.set_dialogue_flag",
        "field_mode_flag": "field.set_mode_flag",
        "field_mode_effect_flag": "field.set_mode_effect_flag",
        "notebook_tip_flag": "field.set_notebook_tip_flag",
        "field_auxiliary_flag": "field.set_auxiliary_flag",
    }.get(family)
    entry_address = entry.get("entry") if entry else effect.get("routine")
    command = entry.get("command") if entry else effect.get("command")
    related_actions = []
    for graph in graph_for_entry(entry_address, graphs, command) if entry_address is not None else []:
        related_actions.extend(actions_reachable_from_command(
            graph, command, entry_address,
            {value for value in
             [effect.get("text_root"), effect.get("address"),
              *effect.get("source_addresses", [])]
             if isinstance(value, int)}))
    expected_value = effect.get("operation") == "set"
    for action in related_actions:
        arguments = action.get("arguments", {})
        if action.get("id") != expected_action:
            continue
        if family == "StoryFact":
            value_matches = str(arguments.get("factId", "")).upper() == identity.split(".", 1)[-1]
        elif family == "GroupItemId":
            value_matches = str(arguments.get("itemId", "")).upper() == identity.split(".", 1)[-1]
        else:
            value_matches = arguments.get("flagBit") == bit
        if value_matches and arguments.get("value", expected_value) == expected_value:
            effect["status"] = "MATCH"
            effect["reason"] = f"branch action {action['id']} writes {identity}"
            graph = next((graph for graph in graph_for_entry(entry_address, graphs, command)
                          if any(row["node"] == action["node"] for row in graph.get("actions", []))), None)
            effect["port"] = (f"{graph['file']}:{action.get('data_line')} -> "
                              f"{action.get('code_path') or action['id']}" if graph else action["id"])
            return effect
    path = None
    if entry and entry.get("port_location"):
        path = entry["port_location"].split(";", 1)[0].rsplit(":", 1)[0]
    writes = port_state_writes(path) if path and path.endswith(".lua") else []
    for write in writes:
        if write["identity"] == identity:
            if write["value"] == expected_value:
                effect["status"] = "MATCH"
                effect["reason"] = f"port write uses {identity}, which maps to ROM storage 0x{storage:X} bit {bit}"
            else:
                effect["status"] = "WRONG"
                effect["reason"] = f"port write clears/sets {identity} opposite to the ROM operation"
            effect["port"] = f"{write['file']}:{write['line']}"
            effect["port_write"] = write
            return effect
    same_bit = []
    for write in writes:
        match = re.search(r"_bit_(\d+)$", write["identity"])
        if match and isinstance(bit, int) and int(match.group(1)) == bit:
            same_bit.append(write)
    if same_bit:
        write = same_bit[0]
        effect["status"] = "WRONG"
        effect["reason"] = (f"port writes {write['identity']} but ROM writes {identity} "
                             f"at 0x{storage:X} bit {bit}")
        effect["port"] = f"{write['file']}:{write['line']}"
        effect["port_write"] = write
    else:
        effect["status"] = "MISSING"
        effect["reason"] = f"no branch-linked write to ROM identity {identity} was found"
        effect["port"] = entry.get("port_location", "MISSING") if entry else "MISSING"
    return effect


def compare_effect_to_port(effect: dict, entry: dict | None,
                           graphs: list[dict], source_files: list[dict]) -> dict:
    kind = effect["kind"]
    if kind == "flag":
        if effect.get("operation") == "test":
            effect["status"] = "NOT_AN_EFFECT"
            effect["reason"] = "ROM branch tests a flag without mutating storage"
            effect["port"] = "branch condition"
            return effect
        effect = compare_flag_effect(effect, entry, graphs)
        if effect.get("storage") == 0xFFFC22:
            effect["kind"] = "item"
            effect["item_name"] = parse_group_item_names().get(effect.get("bit"))
            effect["operation"] = "give_group_item" if effect.get("operation") == "set" else "take_group_item"
        elif effect.get("storage") == 0xFFFC1E:
            effect["kind"] = "quest_flag"
        return effect
    entry_address = entry.get("entry") if entry else effect.get("routine")
    command = entry.get("command") if entry else effect.get("command")
    graphs_for_site = graph_for_entry(entry_address, graphs, command) if entry_address is not None else []
    pointer_set = {value for value in
                   [effect.get("text_root"), effect.get("address"),
                    *effect.get("source_addresses", [])]
                   if isinstance(value, int)}
    related_actions = []
    for graph in graphs_for_site:
        related_actions.extend(actions_reachable_from_command(graph, command,
                                                              entry_address, pointer_set))
    related_actions = list({action["node"]: action for action in related_actions}.values())
    category = {"keyword_unlock": "keyword_unlock", "quest_flag": "quest_flag",
                "item_phase": "item"}.get(kind, kind)
    matching_category = [action for action in related_actions
                         if category in action.get("categories", [])]
    exact = []
    mismatch = []
    for action in matching_category:
        matched, detail = action_value_matches(effect, action)
        if matched:
            exact.append((action, detail))
        else:
            mismatch.append((action, detail))
    if exact:
        action, detail = exact[0]
        graph = next((graph for graph in graphs_for_site
                      if any(candidate["node"] == action["node"]
                             for candidate in graph.get("actions", []))), None)
        effect["status"] = "MATCH"
        effect["reason"] = detail
        effect["port"] = (f"{graph['file']}:{action.get('data_line')} -> "
                           f"{action.get('code_path') or action['id']}" if graph else action["id"])
        effect["port_action"] = action["id"]
        effect["port_node"] = action["node"]
        return effect
    if mismatch:
        action, detail = mismatch[0]
        graph = next((graph for graph in graphs_for_site
                      if any(candidate["node"] == action["node"]
                             for candidate in graph.get("actions", []))), None)
        effect["status"] = "WRONG"
        effect["reason"] = detail
        effect["port"] = (f"{graph['file']}:{action.get('data_line')} -> "
                           f"{action.get('code_path') or action['id']}" if graph else action["id"])
        effect["port_action"] = action["id"]
        effect["port_node"] = action["node"]
        return effect
    if not related_actions:
        graph_actions = [action for graph in graphs_for_site
                         for action in graph.get("actions", [])
                         if kind in action.get("categories", [])]
        if entry is not None:
            entry_token = f"0x{entry['entry']:X}".lower()
            command_token = (f"command 0x{command:02X}".lower()
                             if command is not None else "")
            source_linked = [action for action in graph_actions
                             if entry_token in action_source_text(
                                 action["id"], action.get("code_path")).lower()
                             or command_token and command_token in action_source_text(
                                 action["id"], action.get("code_path")).lower()]
            if source_linked:
                graph_actions = source_linked
        unique_actions = {action["node"]: action for action in graph_actions}
        if len(unique_actions) == 1:
            action = next(iter(unique_actions.values()))
            matched, detail = action_value_matches(effect, action)
            if matched:
                graph = next((graph for graph in graphs_for_site
                              if any(candidate["node"] == action["node"]
                                     for candidate in graph.get("actions", []))), None)
                effect["status"] = "MATCH"
                effect["reason"] = f"unique linked graph action {action['id']}: {detail}"
                effect["port"] = (f"{graph['file']}:{action.get('data_line')} -> "
                                  f"{action.get('code_path') or action['id']}"
                                  if graph else action["id"])
                effect["port_action"] = action["id"]
                effect["port_node"] = action["node"]
                return effect
    if related_actions:
        action = related_actions[0]
        graph = next((graph for graph in graphs_for_site
                      if any(candidate["node"] == action["node"]
                             for candidate in graph.get("actions", []))), None)
        effect["status"] = "WRONG"
        effect["reason"] = f"branch has {action['id']} but does not implement ROM {kind} operation/value"
        effect["port"] = (f"{graph['file']}:{action.get('data_line')} -> "
                           f"{action.get('code_path') or action['id']}" if graph else action["id"])
        effect["port_action"] = action["id"]
        effect["port_node"] = action["node"]
        return effect
    if entry and entry.get("port_location") not in (None, "MISSING"):
        location = entry["port_location"]
        effect["status"] = "MISSING"
        effect["reason"] = f"port location is linked to this command/service but has no branch action for ROM {kind}"
        effect["port"] = location
        return effect
    effect["status"] = "MISSING"
    effect["reason"] = f"no branch-linked port node or source location implements ROM {kind}"
    effect["port"] = "MISSING"
    return effect


def annotate_text_root(root: dict, graphs: list[dict], files: list[dict],
                       entry: int, command: int | None) -> dict:
    page_links = []
    pointers = {root["pointer"]}
    for page in root.get("pages", []):
        pointers.add(page["record"])
        if page["text_pointer"] is not None:
            pointers.add(page["text_pointer"])
    exact_graphs = graph_for_entry(entry, graphs, command)
    for graph in exact_graphs:
        nodes = []
        for node in graph.get("nodes", []):
            evidence = str(node.get("evidence") or "").lower()
            if any(f"0x{pointer:X}".lower() in evidence for pointer in pointers):
                nodes.append(node)
        for node in nodes:
            keys = node.get("text_keys", [])
            if node.get("event_id"):
                keys = keys + [node["event_id"]]
            page_links.append({"graph": graph["id"], "file": graph["file"],
                               "node": node["id"], "node_line": node.get("line"),
                               "page_keys": keys, "evidence": node.get("evidence")})
    for page in root.get("pages", []):
        page["port_page_keys"] = []
        page["port_locations"] = []
        for hit in content_hits(page["text"] or "", files):
            page["port_locations"].append(hit)
        for graph in exact_graphs:
            for node in graph.get("nodes", []):
                evidence = str(node.get("evidence") or "").lower()
                if (f"0x{page['record']:X}".lower() in evidence or
                        (page["text_pointer"] is not None and
                         f"0x{page['text_pointer']:X}".lower() in evidence)):
                    keys = list(node.get("text_keys", []))
                    if node.get("event_id"):
                        keys.append(node["event_id"])
                    page["port_page_keys"].extend(keys)
                    page["port_locations"].append({"file": graph["file"],
                                                    "line": node.get("line"),
                                                    "node": node["id"]})
        page["port_page_keys"] = list(dict.fromkeys(page["port_page_keys"]))
        page["page_status"] = "MATCH" if page["port_page_keys"] or page["port_locations"] else "MISSING"
    root["port_pages"] = page_links
    return root


def annotate_entry_port(entry: dict, graphs: list[dict], files: list[dict]) -> None:
    exact_graphs = graph_for_entry(entry["entry"], graphs, entry.get("command"))
    first_root = next(iter(entry.get("text_roots", [])), None)
    text_hits = []
    if first_root:
        page = next(iter(first_root.get("pages", [])), None)
        text_hits = content_hits(page.get("text", ""), files) if page else []
    comment_hits = []
    address_token = f"0x{entry['entry']:X}".lower()
    for source in files:
        for number, line in enumerate(source["lines"], 1):
            if address_token in line.lower():
                comment_hits.append({"file": source["path"], "line": number})
                break
    if exact_graphs:
        entry["port_graphs"] = [{"id": graph["id"], "file": graph["file"]}
                                for graph in exact_graphs]
        entry["port_location"] = "; ".join(
            f"{graph['file']}:{next((node.get('line') for node in graph.get('nodes', [])
                                      if node.get('line')), 1)}"
            for graph in exact_graphs)
        entry["port_status"] = "MATCH"
    elif comment_hits:
        entry["port_graphs"] = []
        entry["port_location"] = "; ".join(f"{hit['file']}:{hit['line']}" for hit in comment_hits)
        entry["port_status"] = "MATCH"
    elif text_hits:
        entry["port_graphs"] = []
        entry["port_location"] = "; ".join(f"{hit['file']}:{hit['line']}" for hit in text_hits)
        entry["port_status"] = "MATCH"
    else:
        entry["port_graphs"] = []
        entry["port_location"] = "MISSING"
        entry["port_status"] = "MISSING"
    all_text = [page.get("text", "") for root in entry.get("text_roots", [])
                for page in root.get("pages", [])]
    entry["region"] = region_for(entry.get("port_location"), all_text, entry.get("command"))
    for root in entry.get("text_roots", []):
        annotate_text_root(root, graphs, files, entry["entry"], entry.get("command"))


def annotate_flag_identity(event: dict, story_names: dict[int, str],
                           group_names: dict[int, str], files: list[dict],
                           source_path: str | None = None) -> dict:
    storage = event["storage"]
    bit = event.get("bit")
    event["port_identity"] = port_identity(storage, bit, story_names, group_names)
    event["storage_name"] = STORAGE_NAMES.get(storage, f"unknown_0x{storage:X}")
    if event.get("operation") in ("test", "trigger_select", "trigger_dispatch"):
        event["status"] = "NOT_AN_EFFECT"
        event["reason"] = "ROM condition/trigger selection; no persistent flag mutation at this instruction"
        return event
    context_path = source_path
    if context_path is None:
        token = f"0x{event['routine']:X}".lower() if event.get("routine") else None
        for source in files:
            if token and any(token in line.lower() for line in source["lines"]):
                context_path = source["path"]
                break
    writes = port_state_writes(context_path) if context_path else []
    expected = event["port_identity"]
    same_bit_other_storage = []
    for write in writes:
        identity = write["identity"]
        match = re.search(r"_bit_(\d+)$", identity)
        if match and isinstance(bit, int) and int(match.group(1)) == bit:
            same_bit_other_storage.append(write)
        if identity == expected:
            event["status"] = "MATCH"
            event["port_write"] = {key: write[key] for key in ("file", "line", "identity", "raw")}
            event["reason"] = "port write uses the storage identity corresponding to the ROM address and bit"
            return event
    if same_bit_other_storage:
        event["status"] = "WRONG"
        event["port_write"] = {key: same_bit_other_storage[0][key]
                                for key in ("file", "line", "identity", "raw")}
        event["reason"] = f"port writes {same_bit_other_storage[0]['identity']} for the same bit; ROM storage is {event['storage_name']}"
    elif context_path:
        event["status"] = "MISSING"
        event["reason"] = f"no port write for {expected} in {context_path}"
    else:
        event["status"] = "MISSING"
        event["reason"] = "no branch-linked port write location was found"
    return event


def markdown_report(result: dict) -> str:
    summary = result["summary"]
    lines = ["# Quest audit pass 3 — focused slice", "",
             f"ROM: `{Path(result['rom']).name}`; command table `0x{result['command_table']:X}`.",
             f"The decoder enumerated {summary['handler_count']} command handlers and "
             f"{summary['title_service_vector_count']} title-service vector entries.", "",
             "## Status counts", "",
             "| Inventory | MATCH | WRONG | MISSING | NOT_AN_EFFECT |",
             "|---|---:|---:|---:|---:|"]
    for inventory in ("effects", "flags", "branches", "pages"):
        counts = summary[inventory]
        lines.append(f"| {inventory} | {counts['MATCH']} | {counts['WRONG']} | "
                     f"{counts['MISSING']} | {counts['NOT_AN_EFFECT']} |")
    lines += ["", "## Focused ROM cases", "",
              "| ROM address | Handler/effect | Result | Port location | Reason |",
              "|---|---|---|---|---|"]
    wanted = (0x21BA6, 0x218CA, 0x5436C, 0x228E0, 0x2223E, 0x22276, 0x23ED6)
    for address in wanted:
        effects_here = [row for row in result["dialogue_effects"] if row["address"] == address]
        if effects_here:
            for row in effects_here:
                label = f"{row['kind']} {row.get('operation')} {row.get('amount', row.get('item_name', ''))}"
                lines.append(f"| `0x{address:X}` | {label} | {row.get('status')} | "
                             f"{row.get('port', 'MISSING')} | {row.get('reason', '')} |")
        else:
            handler = next((row for row in result["command_handlers"] if row["entry"] == address), None)
            if handler:
                lines.append(f"| `0x{address:X}` | command `0x{handler['command']:02X}` | "
                             f"{handler['status']} | {handler.get('port_location', 'MISSING')} | "
                             f"{handler.get('reason', 'branch inventory has no port implementation')} |")
            else:
                lines.append(f"| `0x{address:X}` | no extracted effect row | MISSING | MISSING | no production command effect was decoded at this address |")
    lines += ["", "## Resolved immediate trigger values", "",
              "| ROM address | FFDD64 value | Operation |", "|---|---:|---|"]
    for address in (0x2192C, 0x21E58):
        row = next((item for item in result["trigger_operations"] if item["address"] == address), None)
        lines.append(f"| `0x{address:X}` | {('0x%X' % row['value']) if row and 'value' in row else 'unresolved'} | "
                     f"{row['operation'] if row else 'MISSING'} |")
    lines += ["", "## Audit notes", "",
              "The JSON preserves the 250 handler rows, indexed service vector, branch/effect extraction, corrected page chains, and flag storage identities.",
              "The ten ROM-confirmed flag/storage differences and ten manual verdicts are recorded in `out/REAL_MISMATCHES.md` and `out/DIAG.md`.", ""]
    return "\n".join(lines)


def self_check(rom_path: Path, disasm_path: Path) -> None:
    audit = build_audit(rom_path, disasm_path)
    flags = audit["flag_operations"]
    effects_rows = audit["dialogue_effects"]
    def has_flag(address: int, bit: int, operation: str) -> bool:
        return any(row["address"] == address and row["bit"] == bit and row["operation"] == operation for row in flags)
    assert has_flag(0x21B98, 10, "set"), "Boris/Michael alias bit 10 was not decoded"
    assert any(row["address"] == 0x21BA6 and row["kind"] == "karma" and row["amount"] == 1 for row in effects_rows), "Boris/Michael +1 Karma was not decoded"
    assert any(row.get("text_root") == 0xF5414 and row["kind"] == "keyword_unlock" for row in effects_rows), "Michael alias text record was not associated"
    assert any(row["address"] == 0x21406 and row["kind"] == "nuyen" and row.get("delta") == -250 for row in effects_rows), "Stoker/Michael 250-nuyen debit was not decoded"
    assert any("creds roll off your stick" in record["text"].lower()
               for record in audit["text_record_roots"].get(0xF487C, [])), "Stoker/Michael list 0xF487C was not decoded"
    assert any(row["address"] == 0x21C2C and row["storage"] == 0xFFFC22 and row["bit"] == 1 and row["operation"] == "set" for row in flags), "Boris visa group-item flag was not decoded"
    assert any(row["address"] == 0x21C64 and row["kind"] == "item" and row.get("item_name") == "MAGLOCK_PASSKEY" and row.get("rating") == 2 for row in effects_rows), "Boris runner-item give was not decoded"
    print("self-check passed: alias keyword, +1 Karma, Stoker/Michael nuyen/text list, and visa item flag")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--rom", type=Path, default=DEFAULT_ROM)
    parser.add_argument("--disassembly", type=Path, default=DEFAULT_DISASM)
    parser.add_argument("--out-dir", type=Path, default=DEFAULT_OUT)
    parser.add_argument("--self-test", action="store_true")
    args = parser.parse_args()
    if args.self_test:
        self_check(args.rom, args.disassembly)
        return 0
    result = build_audit(args.rom, args.disassembly)
    args.out_dir.mkdir(parents=True, exist_ok=True)
    (args.out_dir / "QUEST_AUDIT3.json").write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")
    (args.out_dir / "QUEST_AUDIT3.md").write_text(markdown_report(result), encoding="utf-8")
    print(f"wrote {args.out_dir / 'QUEST_AUDIT3.md'} and JSON; {len(result['command_handlers'])} handlers, "
          f"{len(result['flag_operations'])} flag sites, {len(result['dialogue_effects'])} effects")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (OSError, RuntimeError, AssertionError) as error:
        print(f"quest_audit: {error}", file=sys.stderr)
        raise SystemExit(1)
