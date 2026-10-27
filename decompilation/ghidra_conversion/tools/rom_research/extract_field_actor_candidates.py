#!/usr/bin/env python3
"""Extract the ROM's gated field pedestrian candidate table.

The ROM branch at 0xA800 gates selection; 0x5BA98 indexes the scene table at
0x1EDB88. Each scene row (0x15..0x2B) points to three one-actor lists. The
branch's only non-clear write to FFE183 samples bound 2 and stores 1 or 2, so
table choice 3 is retained as source data but is unreachable after valid field
initialization. The output records source candidates, not three simultaneously
active NPCs. This offline importer writes a named TSV; the game and map viewer
consume the TSV, never the ROM.
"""

from __future__ import annotations

import argparse
import sys
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[3]
DEFAULT_ROM = REPO_ROOT / "input" / "shadowrun_genesis_us.bin"
DEFAULT_OUTPUT = (
    REPO_ROOT
    / "assets"
    / "field"
    / "entities"
    / "conditional-actor-candidates.tsv"
)

SCENE_TABLE = 0x1EDB88
FIRST_SCENE = 0x15
LAST_SCENE = 0x2B
CHOICE_COUNT = 3
PEDESTRIAN_TEMPLATE = 0x1D6988
EXPECTED_OVERRIDES = (0x9E, 0x2E, 0x9E)
ACTIVATION_GATE = (
    "FFFBED=5(after F3DC6 maps source initializer IDs 5/11/12 to 5; 9/10 to 4);"
    "world_mode=2;"
    "FFE12C==FFF0C8;FFFC3C=0;FFE183 indexes table 1..3;zero triggers "
    "0x0D3A bound-2 sample + 1 (fresh choices 1..2; choice 3 unreachable)"
)

FIELDS = (
    "scene",
    "choice",
    "actor_name",
    "entity_kind",
    "palette_line",
    "template_offset",
    "list_offset",
    "position_offset",
    "world_x",
    "world_y",
    "heading_word",
    "direction",
    "source_spawn_state_flags",
    "ordered_overrides",
    "selection_group",
    "activation_gate",
)


def read_word(rom: bytes, offset: int) -> int:
    if offset < 0 or offset + 2 > len(rom):
        raise ValueError(f"word address outside ROM: 0x{offset:X}")
    return int.from_bytes(rom[offset : offset + 2], "big")


def read_long(rom: bytes, offset: int) -> int:
    if offset < 0 or offset + 4 > len(rom):
        raise ValueError(f"long address outside ROM: 0x{offset:X}")
    return int.from_bytes(rom[offset : offset + 4], "big")


def checked_rom_pointer(rom: bytes, offset: int, *, size: int = 1) -> int:
    if offset < 0 or offset + size > len(rom):
        raise ValueError(f"ROM pointer outside image: 0x{offset:X}")
    return offset


def direction_from_heading(heading: int) -> int:
    """Mirror the source 0xB36A rotate/round direction bucket."""
    rotated = ((heading << 3) | (heading >> 13)) & 0xFFFF
    if rotated & 0x8000:
        rotated = (rotated + 1) & 0xFFFF
    return rotated & 0x7


def parse_candidate(rom: bytes, scene: int, choice: int, list_offset: int) -> dict[str, str]:
    tagged_template = read_long(rom, list_offset)
    if not tagged_template & 0x80000000:
        raise ValueError(
            f"scene 0x{scene:02X} choice {choice}: template is not root-tagged"
        )
    template_offset = tagged_template & 0x7FFFFFFF
    checked_rom_pointer(rom, template_offset, size=0x4A)
    if template_offset != PEDESTRIAN_TEMPLATE:
        raise ValueError(
            f"scene 0x{scene:02X} choice {choice}: unexpected template "
            f"0x{template_offset:X}"
        )

    entity_kind = read_word(rom, template_offset)
    if entity_kind != 11:
        raise ValueError(
            f"scene 0x{scene:02X} choice {choice}: expected pedestrian kind 11, "
            f"got {entity_kind}"
        )
    template_flags = read_word(rom, template_offset + 2)
    palette_line = (template_flags >> 13) & 0x3
    position_offset = read_long(rom, list_offset + 4)
    checked_rom_pointer(rom, position_offset, size=4)
    world_x = read_word(rom, position_offset)
    world_y = read_word(rom, position_offset + 2)

    if read_long(rom, list_offset + 8) != 2:
        raise ValueError(
            f"scene 0x{scene:02X} choice {choice}: missing override marker"
        )
    cursor = list_offset + 12
    override_pairs: list[tuple[int, int]] = []
    while True:
        pair = read_word(rom, cursor)
        cursor += 2
        if pair == 0:
            break
        override_pairs.append((pair >> 8, pair & 0xFF))
    if tuple(offset for offset, _ in override_pairs) != EXPECTED_OVERRIDES:
        raise ValueError(
            f"scene 0x{scene:02X} choice {choice}: override order changed: "
            f"{override_pairs!r}"
        )
    if override_pairs[0][1] != 0x20 or override_pairs[2][1] != 0x08:
        raise ValueError(
            f"scene 0x{scene:02X} choice {choice}: actor state overrides changed"
        )
    # 0x44BC applies byte overrides in order. Publish the final +0x9E value as
    # a typed actor-state field so runtime code does not decode ROM offsets.
    source_spawn_state_flags = override_pairs[2][1]
    heading_byte = override_pairs[1][1]
    heading_word = heading_byte << 8

    # The list contains one actor, then a null long sentinel after the ordered
    # byte overrides' zero-word terminator.
    if read_long(rom, cursor) != 0:
        raise ValueError(
            f"scene 0x{scene:02X} choice {choice}: candidate list is not one actor"
        )

    overrides = ",".join(
        f"0x{offset:02X}=0x{value:02X}" for offset, value in override_pairs
    )
    return {
        "scene": str(scene),
        "choice": str(choice),
        "actor_name": "Pedestrian",
        "entity_kind": str(entity_kind),
        "palette_line": str(palette_line),
        "template_offset": f"0x{template_offset:X}",
        "list_offset": f"0x{list_offset:X}",
        "position_offset": f"0x{position_offset:X}",
        "world_x": str(world_x),
        "world_y": str(world_y),
        "heading_word": str(heading_word),
        "direction": str(direction_from_heading(heading_word)),
        "source_spawn_state_flags": str(source_spawn_state_flags),
        "ordered_overrides": overrides,
        "selection_group": f"scene-{scene:02d}-one-of-three",
        "activation_gate": ACTIVATION_GATE,
    }


def extract(rom: bytes) -> list[dict[str, str]]:
    rows: list[dict[str, str]] = []
    for scene in range(FIRST_SCENE, LAST_SCENE + 1):
        scene_pointer_address = SCENE_TABLE + (scene - FIRST_SCENE) * 4
        scene_pointer = read_long(rom, scene_pointer_address) & 0x7FFFFFFF
        checked_rom_pointer(rom, scene_pointer, size=CHOICE_COUNT * 4)
        for choice in range(1, CHOICE_COUNT + 1):
            list_offset = read_long(rom, scene_pointer + (choice - 1) * 4)
            checked_rom_pointer(rom, list_offset, size=24)
            rows.append(parse_candidate(rom, scene, choice, list_offset))
    expected_rows = (LAST_SCENE - FIRST_SCENE + 1) * CHOICE_COUNT
    if len(rows) != expected_rows:
        raise ValueError(f"expected {expected_rows} candidates, found {len(rows)}")
    return rows


def render(rows: list[dict[str, str]]) -> str:
    lines = ["\t".join(FIELDS)]
    lines.extend("\t".join(row[field] for field in FIELDS) for row in rows)
    return "\n".join(lines) + "\n"


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--rom", type=Path, default=DEFAULT_ROM)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument(
        "--check",
        action="store_true",
        help="verify the checked-in TSV matches a fresh ROM extraction",
    )
    args = parser.parse_args()
    try:
        rows = extract(args.rom.read_bytes())
        content = render(rows)
    except (OSError, ValueError) as error:
        print(f"actor candidate extraction failed: {error}", file=sys.stderr)
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
        print(f"verified {len(rows)} ROM candidates")
        return 0

    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(content, encoding="utf-8", newline="\n")
    print(f"wrote {len(rows)} candidates to {args.output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
