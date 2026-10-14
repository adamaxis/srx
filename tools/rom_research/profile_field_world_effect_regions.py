#!/usr/bin/env python3
"""Export the ROM's coordinate-triggered map-effect profiles.

This is an offline source profiler. The native runtime does not read the ROM;
the outputs record the region and actor-list data consumed by the field map
effect path for scenes 20 through 43.
"""

from __future__ import annotations

import argparse
import csv
from collections import defaultdict
from pathlib import Path
import sys


SCENE_TABLE = 0x3232
SCENE_COUNT = 56
FIRST_SCENE = 20
LAST_SCENE = 43
PROFILE_COPY_BYTES = 0x60
PROFILE_EFFECT_POINTER_OFFSET = 0x58
EFFECT_REGION_POINTER_OFFSET = 0x0C
EFFECT_ACTOR_LIST_POINTER_OFFSET = 0x10
REGION_RECORD_BYTES = 8
ACTOR_RECORD_BYTES = 8


def be16(data: bytes, offset: int) -> int:
    return int.from_bytes(data[offset : offset + 2], "big")


def be32(data: bytes, offset: int) -> int:
    return int.from_bytes(data[offset : offset + 4], "big")


def address(value: int) -> str:
    return f"0x{value:06X}"


def longword(value: int) -> str:
    return f"0x{value:08X}"


def read_tsv(path: Path) -> list[dict[str, str]]:
    with path.open("r", encoding="utf-8-sig", newline="") as stream:
        return list(csv.DictReader(stream, delimiter="\t"))


def write_tsv(path: Path, header: list[str], rows: list[list[str]]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w", encoding="utf-8", newline="") as stream:
        writer = csv.writer(stream, delimiter="\t", lineterminator="\n")
        writer.writerow(header)
        writer.writerows(rows)


def require_rom_range(rom: bytes, address_value: int, size: int, context: str) -> None:
    if address_value <= 0 or address_value + size > len(rom):
        raise RuntimeError(
            f"{context}: source address {address_value:#x} is outside the ROM"
        )


def main() -> int:
    repo = Path(__file__).resolve().parents[2]
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--rom",
        type=Path,
        default=repo / "input/shadowrun_genesis_us.bin",
    )
    parser.add_argument(
        "--assets",
        type=Path,
        default=repo / "assets",
    )
    parser.add_argument(
        "--docs",
        type=Path,
        default=repo / "docs/ghidra/ghidra",
    )
    args = parser.parse_args()

    rom = args.rom.read_bytes()
    scene_profiles = {
        scene: be32(rom, SCENE_TABLE + scene * 4)
        for scene in range(SCENE_COUNT)
    }
    profile_alias_groups: dict[int, list[int]] = defaultdict(list)
    for scene, profile in scene_profiles.items():
        profile_alias_groups[profile].append(scene)

    effect_profile_addresses = {
        scene: be32(
            rom,
            scene_profiles[scene] + PROFILE_EFFECT_POINTER_OFFSET,
        )
        for scene in range(FIRST_SCENE, LAST_SCENE + 1)
    }
    effect_profile_alias_groups: dict[int, list[int]] = defaultdict(list)
    effect_actor_list_alias_groups: dict[int, list[int]] = defaultdict(list)
    for scene, effect_profile in effect_profile_addresses.items():
        if not effect_profile:
            continue
        require_rom_range(rom, effect_profile, 0x14, f"scene {scene} effect profile")
        effect_profile_alias_groups[effect_profile].append(scene)
        actor_list = be32(
            rom,
            effect_profile + EFFECT_ACTOR_LIST_POINTER_OFFSET,
        )
        effect_actor_list_alias_groups[actor_list].append(scene)

    actor_rows = read_tsv(args.assets / "field/entities/default-actors.tsv")
    actor_templates: dict[int, set[tuple[str, str, str]]] = defaultdict(set)
    for row in actor_rows:
        template = int(row["template_offset"], 16)
        actor_templates[template].add(
            (row["actor_name"], row["entity_kind"], row["sprite_stem"])
        )

    profile_rows: list[list[str]] = []
    region_rows: list[list[str]] = []
    actor_list_rows: list[list[str]] = []
    region_total = 0
    actor_total = 0
    scene_profiles_with_regions = 0
    scene_profiles_with_actors = 0

    for scene in range(FIRST_SCENE, LAST_SCENE + 1):
        profile = scene_profiles[scene]
        require_rom_range(rom, profile, PROFILE_COPY_BYTES, f"scene {scene} profile")
        effect_pointer_address = profile + PROFILE_EFFECT_POINTER_OFFSET
        effect_profile = be32(rom, effect_pointer_address)
        aliases = profile_alias_groups[profile]
        alias_text = ",".join(map(str, aliases)) if len(aliases) > 1 else ""

        if effect_profile == 0:
            if scene != 20:
                raise RuntimeError(
                    f"scene {scene}: expected a world-effect profile extension"
                )
            profile_rows.append(
                [
                    str(scene), address(SCENE_TABLE + scene * 4), address(profile),
                    alias_text, "", address(effect_pointer_address), "", "", "", "", "0", "0",
                    "scene 20 schedules the world-activity timeout; no map actor list",
                ]
            )
            continue

        require_rom_range(rom, effect_profile, 0x14, f"scene {scene} effect profile")
        region_pointer = be32(rom, effect_profile + EFFECT_REGION_POINTER_OFFSET)
        actor_list_pointer = be32(rom, effect_profile + EFFECT_ACTOR_LIST_POINTER_OFFSET)
        require_rom_range(rom, region_pointer, 2, f"scene {scene} region list")
        require_rom_range(rom, actor_list_pointer, 4, f"scene {scene} actor list")

        regions: list[tuple[int, int, int, int, int, int]] = []
        cursor = region_pointer
        for index in range(256):
            require_rom_range(rom, cursor, REGION_RECORD_BYTES, f"scene {scene} region {index}")
            x_min = be16(rom, cursor)
            if x_min & 0x8000:
                break
            x_max = be16(rom, cursor + 2)
            y_min = be16(rom, cursor + 4)
            y_max = be16(rom, cursor + 6)
            regions.append((index, cursor, x_min, x_max, y_min, y_max))
            cursor += REGION_RECORD_BYTES
        else:
            raise RuntimeError(f"scene {scene}: region list has no signed sentinel")

        actor_entries: list[
            tuple[int, int, int, int, int, int, int, str, str, str]
        ] = []
        cursor = actor_list_pointer
        for index in range(64):
            require_rom_range(rom, cursor, ACTOR_RECORD_BYTES, f"scene {scene} actor {index}")
            tagged_template = be32(rom, cursor)
            position_pointer = be32(rom, cursor + 4)
            if tagged_template == 0:
                break
            template = tagged_template & 0x7FFFFFFF
            require_rom_range(rom, template, 2, f"scene {scene} template {index}")
            require_rom_range(rom, position_pointer, 6, f"scene {scene} position {index}")
            world_x = be16(rom, position_pointer + 2)
            world_y = be16(rom, position_pointer + 4)
            labels = actor_templates.get(template, set())
            if len(labels) == 1:
                actor_name, entity_kind, sprite_stem = next(iter(labels))
            elif len(labels) > 1:
                raise RuntimeError(
                    f"scene {scene}: template {address(template)} has ambiguous labels"
                )
            else:
                actor_name, entity_kind, sprite_stem = (
                    "unmatched_template",
                    "",
                    "",
                )
            actor_entries.append(
                (
                    index,
                    cursor,
                    tagged_template,
                    template,
                    position_pointer,
                    world_x,
                    world_y,
                    actor_name,
                    entity_kind,
                    sprite_stem,
                )
            )
            cursor += ACTOR_RECORD_BYTES
        else:
            raise RuntimeError(f"scene {scene}: actor list has no null sentinel")

        if regions:
            scene_profiles_with_regions += 1
        if actor_entries:
            scene_profiles_with_actors += 1
        region_total += len(regions)
        actor_total += len(actor_entries)

        profile_rows.append(
            [
                str(scene),
                address(SCENE_TABLE + scene * 4),
                address(profile),
                alias_text,
                ",".join(map(str, effect_profile_alias_groups[effect_profile]))
                if len(effect_profile_alias_groups[effect_profile]) > 1
                else "",
                address(effect_pointer_address),
                address(effect_profile),
                address(region_pointer),
                address(actor_list_pointer),
                ",".join(map(str, effect_actor_list_alias_groups[actor_list_pointer]))
                if len(effect_actor_list_alias_groups[actor_list_pointer]) > 1
                else "",
                str(len(regions)),
                str(len(actor_entries)),
                "0x1DFC4 coordinate check; 0x1E04A explicit alarm transition",
            ]
        )

        for index, record, x_min, x_max, y_min, y_max in regions:
            region_rows.append(
                [
                    str(scene), address(profile), address(effect_profile),
                    address(region_pointer), str(index), address(record),
                    str(x_min), str(x_max), str(y_min), str(y_max),
                    "exclusive", "inclusive", "exclusive", "inclusive",
                ]
            )

        for (
            index,
            record,
            tagged_template,
            template,
            position_pointer,
            world_x,
            world_y,
            actor_name,
            entity_kind,
            sprite_stem,
        ) in actor_entries:
            actor_list_rows.append(
                [
                    str(scene), address(profile), address(effect_profile),
                    address(actor_list_pointer), str(index), address(record),
                    longword(tagged_template),
                    address(template), str(bool(tagged_template & 0x80000000)).lower(),
                    actor_name, entity_kind, sprite_stem, address(position_pointer),
                    str(world_x), str(world_y), str(world_x >> 4), str(world_y >> 4),
                ]
            )

    profile_path = args.docs / "field-world-effect-profiles.tsv"
    region_path = args.docs / "field-world-effect-regions.tsv"
    actor_path = args.docs / "field-world-effect-actors.tsv"
    write_tsv(
        profile_path,
        [
            "scene_id", "scene_table_entry", "profile_address",
            "shared_profile_scene_ids", "shared_effect_profile_scene_ids",
            "effect_pointer_address",
            "effect_profile_address", "region_list_address", "actor_list_address",
            "shared_actor_list_scene_ids", "region_count", "actor_count",
            "source_path",
        ],
        profile_rows,
    )
    write_tsv(
        region_path,
        [
            "scene_id", "profile_address", "effect_profile_address",
            "region_list_address", "region_index", "record_address",
            "x_min_pixels", "x_max_pixels", "y_min_pixels", "y_max_pixels",
            "x_min_bound", "x_max_bound", "y_min_bound", "y_max_bound",
        ],
        region_rows,
    )
    write_tsv(
        actor_path,
        [
            "scene_id", "profile_address", "effect_profile_address",
            "actor_list_address", "source_order", "record_address",
            "tagged_template_raw", "template_address",
            "starts_actor_group", "actor_name", "entity_kind", "sprite_stem",
            "position_record_address", "world_x_12_4", "world_y_12_4",
            "world_x_pixels", "world_y_pixels",
        ],
        actor_list_rows,
    )

    print(f"Profiled scene IDs {FIRST_SCENE}-{LAST_SCENE} from the US ROM.")
    print(
        f"Found {scene_profiles_with_regions} profiles with coordinate regions "
        f"({region_total} rectangles) and {scene_profiles_with_actors} profiles "
        f"with actor lists ({actor_total} entries)."
    )
    print(f"Wrote {profile_path}")
    print(f"Wrote {region_path}")
    print(f"Wrote {actor_path}")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (OSError, RuntimeError, ValueError, IndexError, KeyError) as error:
        print(f"profile_field_world_effect_regions: {error}", file=sys.stderr)
        raise SystemExit(1)
