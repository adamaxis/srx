#!/usr/bin/env python3
"""Extract conditional field dialogue outcomes selected by the US ROM.

The selector at ROM 0x59986 maps each of 56 field contexts to one of 16
groups, rolls twice with bound five, and indexes the 16x9 enable table at
0x1D70E4. Enabled cells indirect through the pointer table at 0x1D7174 to
one of the ROM's field-event handlers. Disabled cells take the separate
dynamic world-effect path. This offline extractor records the outcome and the
opening dialogue/menu text; it does not place a character on the map.
"""

from __future__ import annotations

import argparse
import csv
import sys
from collections import Counter
from pathlib import Path


DECOMPILATION = Path(__file__).resolve().parents[1]
DEFAULT_ROM = DECOMPILATION / "input" / "shadowrun_genesis_us.bin"
DEFAULT_OUTPUT = DECOMPILATION / "assets" / "field" / "world-activity-outcomes.tsv"

CONTEXT_GROUP_TABLE = 0x3312
CONTEXT_COUNT = 56
OUTCOME_MASK_TABLE = 0x1D70E4
OUTCOME_POINTER_TABLE = 0x1D7174
OUTCOMES_PER_GROUP = 9
RANDOM_BOUND = 5
GENERIC_OUTCOME = "dynamic_world_effect_sequence"
ACTIVATION = (
    "Eligible field-loop pass at tick counter modulo 1024 == 10, after the "
    "ROM effect/entity guards and source random-threshold check; two random "
    "rolls with bound 5 select a sum from 0 through 8."
)

FIELDS = (
    "scene",
    "context_group",
    "random_sum",
    "roll_pairs",
    "roll_pair_count",
    "outcome_kind",
    "event_name",
    "handler_offset",
    "prompt_record_offset",
    "resource_bundle_offset",
    "opening_text",
    "choice_1",
    "choice_2",
    "choice_3",
    "map_position",
    "activation",
    "selection_cell_offset",
    "selection_pointer_offset",
    "selected_target_offset",
)


def read_u32(rom: bytes, offset: int) -> int:
    if offset < 0 or offset + 4 > len(rom):
        raise ValueError(f"longword outside ROM at 0x{offset:X}")
    return int.from_bytes(rom[offset : offset + 4], "big")


def decode_dialogue(rom: bytes, offset: int) -> str:
    """Decode the game's printable dialogue stream and paragraph controls."""
    if offset < 0 or offset >= len(rom):
        raise ValueError(f"dialogue pointer outside ROM: 0x{offset:X}")
    segments: list[str] = [""]
    for cursor in range(offset, min(len(rom), offset + 4096)):
        byte = rom[cursor]
        if byte == 0xFF:
            return "|".join(segments).strip()
        if byte >= 0x80:
            segments.append("")
        elif 0x20 <= byte <= 0x7E:
            segments[-1] += chr(byte)
        else:
            raise ValueError(
                f"unsupported dialogue byte 0x{byte:02X} at 0x{cursor:X}"
            )
    raise ValueError(f"dialogue at 0x{offset:X} has no ROM terminator")


def prompt_fields(rom: bytes, handler: int) -> tuple[int, int, list[str]]:
    """Read the absolute prompt/resource operands used by a handler stub."""
    if rom[handler : handler + 2] != b"\x41\xF9":
        raise ValueError(f"handler 0x{handler:X} does not begin with LEA A0")
    prompt = read_u32(rom, handler + 2)
    cursor = handler + 6
    resource_bundle = 0
    if rom[cursor : cursor + 2] == b"\x4B\xF9":
        resource_bundle = read_u32(rom, cursor + 2)

    text_fields: list[str] = []
    for index in range(4):
        pointer = read_u32(rom, prompt + index * 4)
        if pointer in (0, 0xFFFFFFFF):
            text_fields.append("")
        else:
            text_fields.append(decode_dialogue(rom, pointer))
    return prompt, resource_bundle, text_fields


def rolls_for_sum(total: int) -> list[str]:
    return [
        f"{left}+{right}"
        for left in range(RANDOM_BOUND)
        for right in range(RANDOM_BOUND)
        if left + right == total
    ]


def extract(rom: bytes) -> list[dict[str, str]]:
    if len(rom) < OUTCOME_POINTER_TABLE + 16 * OUTCOMES_PER_GROUP * 4:
        raise ValueError("ROM is too short for the world activity tables")
    rows: list[dict[str, str]] = []
    handler_cache: dict[int, tuple[int, int, list[str]]] = {}
    handler_names: Counter[int] = Counter()
    enabled_cells = 0
    for scene in range(CONTEXT_COUNT):
        group = rom[CONTEXT_GROUP_TABLE + scene]
        if group >= 16:
            raise ValueError(f"scene {scene} has invalid context group {group}")
        for total in range(OUTCOMES_PER_GROUP):
            cell_index = group * OUTCOMES_PER_GROUP + total
            cell_offset = OUTCOME_MASK_TABLE + cell_index
            pointer_offset = OUTCOME_POINTER_TABLE + cell_index * 4
            selected_pointer = read_u32(rom, pointer_offset)
            enabled = rom[cell_offset]
            if enabled not in (0, 1):
                raise ValueError(
                    f"selection cell at 0x{cell_offset:X} is {enabled}, expected 0 or 1"
                )

            if enabled:
                enabled_cells += 1
                handler = read_u32(rom, selected_pointer)
                if handler not in handler_cache:
                    handler_cache[handler] = prompt_fields(rom, handler)
                prompt, resource_bundle, text_fields = handler_cache[handler]
                first_text = text_fields[0]
                event_name = first_text.split("|", 1)[0].strip(" <>\t")
                if not event_name:
                    event_name = f"ROM field event at {handler:06X}"
                outcome_kind = "dialogue_event"
                handler_offset = f"0x{handler:X}"
                prompt_offset = f"0x{prompt:X}"
                resource_offset = (
                    f"0x{resource_bundle:X}" if resource_bundle else ""
                )
                target_offset = handler_offset
                opening, choice_a, choice_b, choice_c = text_fields
                handler_names[handler] += 1
            else:
                outcome_kind = GENERIC_OUTCOME
                event_name = "Dynamic world actor/effect sequence"
                handler_offset = ""
                prompt_offset = ""
                resource_offset = ""
                target_offset = ""
                opening = choice_a = choice_b = choice_c = ""

            pairs = rolls_for_sum(total)
            rows.append({
                "scene": str(scene),
                "context_group": str(group),
                "random_sum": str(total),
                "roll_pairs": ", ".join(pairs),
                "roll_pair_count": str(len(pairs)),
                "outcome_kind": outcome_kind,
                "event_name": event_name,
                "handler_offset": handler_offset,
                "prompt_record_offset": prompt_offset,
                "resource_bundle_offset": resource_offset,
                "opening_text": opening,
                "choice_1": choice_a,
                "choice_2": choice_b,
                "choice_3": choice_c,
                "map_position": "not fixed by this selector",
                "activation": ACTIVATION,
                "selection_cell_offset": f"0x{cell_offset:X}",
                "selection_pointer_offset": f"0x{pointer_offset:X}",
                "selected_target_offset": target_offset,
            })

    if len(rows) != CONTEXT_COUNT * OUTCOMES_PER_GROUP:
        raise ValueError(f"expected 504 scene/sum outcomes, extracted {len(rows)}")
    if len(handler_cache) != 26:
        raise ValueError(
            f"expected 26 unique handlers, found {len(handler_cache)}"
        )
    if enabled_cells != sum(handler_names.values()):
        raise ValueError("enabled outcome count does not match handler references")
    return rows


def render(rows: list[dict[str, str]]) -> str:
    from io import StringIO

    stream = StringIO(newline="")
    writer = csv.DictWriter(
        stream, fieldnames=FIELDS, delimiter="\t", lineterminator="\n"
    )
    writer.writeheader()
    writer.writerows(rows)
    return stream.getvalue()


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--rom", type=Path, default=DEFAULT_ROM)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument(
        "--check", action="store_true",
        help="verify the checked-in TSV against a fresh ROM extraction",
    )
    args = parser.parse_args()
    try:
        content = render(extract(args.rom.read_bytes()))
    except (OSError, ValueError) as error:
        print(f"world activity extraction failed: {error}", file=sys.stderr)
        return 1
    if args.check:
        try:
            existing = args.output.read_text(encoding="utf-8")
        except OSError as error:
            print(f"cannot read {args.output}: {error}", file=sys.stderr)
            return 1
        if existing != content:
            print(f"ROM extraction differs from {args.output}", file=sys.stderr)
            return 1
        print("verified 504 scene/sum outcomes and 26 ROM handlers")
        return 0
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(content, encoding="utf-8", newline="\n")
    print(f"wrote 504 scene/sum outcomes to {args.output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
