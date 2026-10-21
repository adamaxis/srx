#!/usr/bin/env python3
"""Profile ROM field collision triggers and verify the extracted source records.

This is an offline audit tool. It reads the US ROM directly, checks the named
field-scene assets against it, and writes TSVs for all scene profiles and all
high-bit collision records. It is not used by the game runtime.
"""

from __future__ import annotations

import argparse
import csv
from collections import Counter, defaultdict
from pathlib import Path
import sys


SCENE_TABLE = 0x3232
SCENE_COUNT = 56
PROFILE_BYTES = 0x58
COLLISION_POINTER_OFFSET = 0x26
EVENT_POINTER_OFFSET = 0x4E
COLLISION_RECORD_BYTES = 10
EVENT_TABLE_BYTES = 32


def be16(data: bytes, offset: int) -> int:
    return int.from_bytes(data[offset : offset + 2], "big")


def be32(data: bytes, offset: int) -> int:
    return int.from_bytes(data[offset : offset + 4], "big")


def address(value: int) -> str:
    return f"0x{value:06X}"


def word(value: int) -> str:
    return f"0x{value:04X}"


def event_handler(event: int, scene: int) -> tuple[str, str, str]:
    if scene == 2 and event == 2:
        return (
            "scene2_stoker_dialogue_override",
            "0xB7B8; 0x1F5CA; 0x1F65A",
            "field_events.lua; GameScriptRuntime; FieldDialogueRuntime; building.lua; FieldSceneRuntime",
        )
    if scene == 2 and event == 11:
        return (
            "scene2_ares_service_override",
            "0xB7B8; 0x1F5CA; 0x1FA78",
            "field_events.lua; GameScriptRuntime; FieldActionRuntime; building.lua; FieldSceneRuntime",
        )
    if scene == 4 and event == 3:
        return (
            "field_mode_4_3_medicine_lodge_service",
            "0xB7B8; 0x1FCF2; 0x56AC0",
            "field_events.lua; GameScriptRuntime; FieldSceneRuntime; council_medicine_lodge.lua",
        )
    if scene == 4 and event == 4:
        return (
            "field_mode_4_4_hospital_service",
            "0xB7B8; 0x1FA02",
            "field_events.lua; GameScriptRuntime; FieldSceneRuntime; council_island_hospital.lua",
        )
    if 21 <= scene <= 43 and event == 0:
        return (
            "resource_activity_source_field_return",
            "0xB7B8; 0x1651E; 0x16E30; 0x2F32",
            "field_events.lua; GameScriptRuntime.resolve_field_location_route; FieldSceneRuntime snapshot/restore_world_activity_source_scene",
        )
    if 2 <= event <= 5 and (scene == 20 or 21 <= scene <= 43):
        return (
            "conditional_electromagnetic_lock",
            "0xB7B8; 0x20E48",
            "field_events.lua; GameScriptRuntime; advance_entity_movement_and_mode_transition; FieldInteractionRuntime",
        )
    direct = {
        0x18: (
            "security_terminal",
            "0xB7B8; 0x20EC2",
            "field_events.lua; GameScriptRuntime; advance_entity_movement_and_mode_transition; FieldInteractionRuntime",
        ),
        0x19: (
            "conditional_event25_or_rewrite_to_event8",
            "0xB7B8; 0x53EFA",
            "field_events.lua; GameScriptRuntime; advance_entity_movement_and_mode_transition; MovementModeTransitionContinuation",
        ),
        0x1A: (
            "projected_terrain_marker_then_activity",
            "0xB7B8; 0x16760; 0x20C90",
            "field_events.lua; GameScriptRuntime; advance_entity_movement_and_mode_transition; mark_projected_terrain_cell; FieldSceneRuntime",
        ),
        0x1B: (
            "world_activity_event27",
            "0xB7B8; 0x53D5A",
            "field_events.lua; GameScriptRuntime; advance_entity_movement_and_mode_transition; MovementModeTransitionContinuation",
        ),
        0x1C: (
            "changed_floor_scene_handoff",
            "0xB7B8; 0x58DB2",
            "field_events.lua; GameScriptRuntime; advance_entity_movement_and_mode_transition; MovementModeTransitionContinuation",
        ),
        0x1D: (
            "floor_selector",
            "0xB7B8; 0x58D94",
            "field_events.lua; GameScriptRuntime; advance_entity_movement_and_mode_transition; ElevatorSelectorRuntime",
        ),
        0x1E: (
            "world_activity_event30",
            "0xB7B8; 0x20990",
            "field_events.lua; GameScriptRuntime; advance_entity_movement_and_mode_transition; MovementModeTransitionContinuation",
        ),
        0x1F: (
            "world_activity_event31",
            "0xB7B8; 0x0DB26",
            "field_events.lua; GameScriptRuntime; advance_entity_movement_and_mode_transition; MovementModeTransitionContinuation",
        ),
    }
    if event in direct:
        return direct[event]
    return (
        "generic_field_event_route",
        "0xB7B8; 0x1651E; 0x1F5CA",
        "field_events.lua via GameScriptRuntime.resolve_field_collision_event; advance_entity_movement_and_mode_transition; apply_world_map_effect; building.lua via GameScriptRuntime.resolve_field_location_route",
    )


def table_route_description(scene: int, event: int, table: bytes) -> tuple[str, str]:
    """Describe the table lookup without treating every byte as a doorway."""
    if 21 <= scene <= 43 and event == 0:
        return (
            "resource_activity_source_field_return",
            "null event table; 0xB7B8 -> 0x1651E starts the field transition, and 0x2F32 restores the saved source scene",
        )
    if scene == 2 and event in (2, 11):
        return "bypassed_by_scene2_story_or_service_route", "table byte is diagnostic only"
    if scene == 4 and event == 3:
        return "bypassed_by_field_mode_4_3_medicine_lodge_service", "same-map service; table byte is diagnostic only"
    if scene == 4 and event == 4:
        return "bypassed_by_field_mode_4_4_hospital_service", "same-map hospital prompt; table byte is diagnostic only"
    if event in (0x18, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F):
        return "bypassed_by_direct_B7B8_continuation", "direct event continuation"
    if event == 0x19:
        return "contextual_event25_or_event8_rewrite", "0x19 only when FFFC10=0x13; otherwise table index 0x08"
    # 0xB7B8 shares these event IDs with the maglock prompt only in scene 20
    # or while FFDF40 selects resource-activity mode (scene IDs 21..43).
    # Ordinary field maps 0..19 publish the event to their event table.
    if 2 <= event <= 5 and (scene == 20 or 21 <= scene <= 43):
        if table:
            return f"conditional_maglock_else_table_index_0x{event:02X}", describe_target(table[event])
        return "conditional_maglock_else_no_table", "no event table on generic fallthrough"
    if not table:
        if event == 0:
            return (
                "null_event_table_index_0x00",
                "ROM address 0x000000 byte 0x00 selects field scene 0x00",
            )
        return "no_event_table", "unresolved generic route"
    return f"event_table_index_0x{event:02X}", describe_target(table[event])


def describe_target(value: int) -> str:
    if value <= 0x14:
        if value == 0x14:
            return "field scene 0x14; 0x167AE flag/timeout gate applies"
        if value == 0x0D:
            return "field scene 0x0D; 0x167AE world-script selection checks FFFC1E bit 19"
        if value == 0x0F:
            return "field scene 0x0F; 0x167AE world-script selection checks FFFC1E bit 29"
        return f"field scene 0x{value:02X}"
    if value < 0x2C:
        return f"resource activity scene 0x{value:02X}"
    if value < 0x38:
        return f"scripted activity scene 0x{value:02X}"
    return f"0x{value:02X} is outside 0x167AE scene ranges; source halts"


def physical_qa_route(scene: int, record_index: int, event: int) -> str:
    if scene == 21 and record_index == 102 and event == 0:
        return (
            "Debug/Release ordinary Down QA enters this resource event-0 cell "
            "and restores the source field at the saved probe lead with the "
            "away-facing heading; see fuchi-offices-scene3-event2.md"
        )
    if scene == 35 and record_index == 121 and event == 0:
        return (
            "Debug/Release ordinary Down QA enters this resource event-0 cell "
            "and restores the source field at the saved probe lead with the "
            "away-facing heading; see renraku-offices-scene5-event6.md"
        )
    if scene == 1 and record_index in (76, 77):
        return (
            "Debug/Release ordinary Up QA reaches the exact active Puyallup "
            "gang target and verifies its passage/Scene-15 actor group; a "
            "mismatched target and no-run case follow row 9 to Scene 13"
        )
    if scene == 3 and record_index == 100:
        return (
            "Debug/Release ordinary Up QA reaches the exact active Eye-Fivers "
            "gang target and verifies its passage/Scene-15 actor group; a "
            "different target and no-run case follow row 4 to Scene 13"
        )
    if scene == 1 and record_index == 69:
        return (
            "Debug/Release ordinary Up route selects scene 13 at zero funds; "
            "ILENE_MET bit 19 gates the post-load actor; see Crime Mall QA"
        )
    if scene == 1 and record_index == 72:
        return (
            "Debug/Release ordinary Up route selects scene 13 at zero funds; "
            "ILENE_MET bit 19 gates the post-load actor; see Riannon's QA"
        )
    if scene == 1 and record_index == 74:
        return (
            "Debug/Release ordinary Up route selects scene 13 at zero funds; "
            "ILENE_MET bit 19 gates the post-load actor; see Dr. Bob's QA"
        )
    if scene == 1 and record_index == 70:
        return (
            "Debug/Release ordinary Up route selects scene 15 at zero funds; "
            "bit 29 gates the post-load actor list; see Underground 93 QA"
        )
    if scene == 13 and record_index in (9, 10):
        return (
            "Debug/Release ordinary Down route enters field scene 0; "
            "see Jump House entry and Gunderson audit"
        )
    if scene == 2 and record_index in (89, 90):
        return "New Game physical route; C0/event 0 selects scene 0x14"
    if scene == 2 and record_index == 91:
        return "New Game physical route; C2 opens Stoker dialogue"
    if scene == 2 and record_index == 92:
        return "Physical Right/Up route; CB opens Ares service"
    if scene == 2 and record_index == 93:
        return "New Game physical route; C1/event 1 selects resource scene 0x1A"
    if scene == 20 and record_index == 117:
        return "Scene-20 physical interaction routes; see scene20-maglock-physical-entry.md"
    if scene == 26 and record_index == 110:
        return "Scene-26 physical passkey route; sampled passage cell 525"
    if scene == 26 and record_index == 117:
        return "Scene-26 physical passkey route; sampled passage cell 712"
    if scene == 26 and record_index in (124, 125):
        return "Joined physical event-1 route reaches Security Terminal D8"
    if scene == 21 and record_index == 121 and event == 0x1F:
        return (
            "Debug/Release after gated Fuchi entry; ordinary Up opens the "
            "resource Main Menu and Cyberspace enters system 22/node 13"
        )
    if scene == 22 and record_index == 136 and event == 0x1F:
        return (
            "Debug/Release after the Scene-25 elevator group roundtrip; "
            "ordinary Up opens the resource Main Menu and Cyberspace enters "
            "system 22/node 13"
        )
    if scene == 24 and record_index == 136 and event == 0x1F:
        return (
            "Debug/Release after the Scene-25 elevator route to Scene 24; "
            "ordinary Up opens the resource Main Menu and Cyberspace enters "
            "system 22/node 8"
        )
    if scene == 26 and record_index == 126 and event == 0x1F:
        return (
            "Debug/Release joined event-1 route; ordinary Up opens the "
            "resource Main Menu and Cyberspace enters system 24/node 22"
        )
    if event == 0x1D:
        return "Physical group trigger QA; see visual-audits/field-elevator-selector.md"
    return "No focused physical-input route recorded for this rectangle"


def read_tsv(path: Path) -> list[dict[str, str]]:
    with path.open("r", encoding="utf-8-sig", newline="") as stream:
        return list(csv.DictReader(stream, delimiter="\t"))


def write_tsv(path: Path, header: list[str], rows: list[list[str]]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w", encoding="utf-8", newline="") as stream:
        writer = csv.writer(stream, delimiter="\t", lineterminator="\n")
        writer.writerow(header)
        writer.writerows(rows)


def main() -> int:
    repo = Path(__file__).resolve().parents[3]
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--rom",
        type=Path,
        default=repo / "input/shadowrun_genesis_us.bin",
    )
    parser.add_argument("--assets", type=Path, default=repo / "assets")
    parser.add_argument(
        "--docs", type=Path, default=repo / "docs/ghidra"
    )
    args = parser.parse_args()

    rom = args.rom.read_bytes()
    scene_index = read_tsv(args.assets / "records/field/scenes/index.tsv")
    indexed = {int(row["scene_id"]): row for row in scene_index}
    if len(indexed) != SCENE_COUNT or set(indexed) != set(range(SCENE_COUNT)):
        raise RuntimeError("scene resource index must contain scene IDs 0..55")

    profile_addresses = {
        scene: be32(rom, SCENE_TABLE + scene * 4)
        for scene in range(SCENE_COUNT)
    }
    profile_alias_groups: dict[int, list[int]] = defaultdict(list)
    collision_alias_groups: dict[int, list[int]] = defaultdict(list)
    collision_addresses = {
        scene: be32(rom, profile + COLLISION_POINTER_OFFSET)
        for scene, profile in profile_addresses.items()
    }
    for scene, profile in profile_addresses.items():
        profile_alias_groups[profile].append(scene)
        collision_alias_groups[collision_addresses[scene]].append(scene)

    profiles: list[list[str]] = []
    trigger_rows: list[list[str]] = []
    totals = Counter()
    event_totals = Counter()
    verified_collision_bytes = 0
    unique_collision_record_addresses: set[int] = set()
    unique_high_bit_record_addresses: set[int] = set()

    for scene in range(SCENE_COUNT):
        table_entry = SCENE_TABLE + scene * 4
        profile = profile_addresses[scene]
        if profile + PROFILE_BYTES > len(rom):
            raise RuntimeError(f"scene {scene}: profile address outside ROM")
        if int(indexed[scene]["record_offset"], 16) != profile:
            raise RuntimeError(f"scene {scene}: extracted profile pointer differs")

        scene_dir = args.assets / "records/field/scenes" / f"scene-{scene:02d}"
        descriptor_asset = (scene_dir / "descriptor.bin").read_bytes()
        descriptor_rom = rom[profile : profile + PROFILE_BYTES]
        if descriptor_asset != descriptor_rom:
            raise RuntimeError(f"scene {scene}: descriptor asset differs from ROM")

        collision_pointer_address = profile + COLLISION_POINTER_OFFSET
        collision_address = collision_addresses[scene]
        event_pointer_address = profile + EVENT_POINTER_OFFSET
        event_address = be32(rom, event_pointer_address)
        event_asset_path = scene_dir / "event-table.bin"
        if event_address:
            event_bytes = rom[event_address : event_address + EVENT_TABLE_BYTES]
            if len(event_bytes) != EVENT_TABLE_BYTES:
                raise RuntimeError(f"scene {scene}: event table truncated")
            if not event_asset_path.is_file() or event_asset_path.read_bytes() != event_bytes:
                raise RuntimeError(f"scene {scene}: event table asset differs from ROM")
        else:
            event_bytes = b""
            if event_asset_path.exists():
                raise RuntimeError(f"scene {scene}: unexpected event table asset")

        records = bytearray()
        parsed: list[tuple[int, int, int, int, int, int, int]] = []
        cursor = collision_address
        record_index = 0
        while True:
            if cursor + 2 > len(rom):
                raise RuntimeError(f"scene {scene}: collision sentinel missing")
            x_start = be16(rom, cursor)
            if x_start & 0x8000:
                sentinel_address = cursor
                break
            if cursor + COLLISION_RECORD_BYTES > len(rom):
                raise RuntimeError(f"scene {scene}: collision record truncated")
            raw = rom[cursor : cursor + COLLISION_RECORD_BYTES]
            records.extend(raw)
            words = tuple(be16(raw, offset) for offset in range(0, 10, 2))
            x0, x1, y0, y1, value = words
            parsed.append((record_index, cursor, x0, x1, y0, y1, value))
            unique_collision_record_addresses.add(cursor)
            cursor += COLLISION_RECORD_BYTES
            record_index += 1

        collision_asset = (scene_dir / "terrain-collision-rectangles.bin").read_bytes()
        if bytes(records) != collision_asset:
            raise RuntimeError(f"scene {scene}: collision records differ from ROM")
        verified_collision_bytes += len(records)
        scene_counts: Counter[int] = Counter()
        event_records = 0
        for index, record_address, x0, x1, y0, y1, value in parsed:
            if value & 0x80:
                event = value & 0x1F
                unique_high_bit_record_addresses.add(record_address)
                event_records += 1
                scene_counts[event] += 1
                event_totals[event] += 1
                totals["event_records"] += 1
                table_byte = event_bytes[event] if event_bytes else None
                family, handler_addresses, native_functions = event_handler(event, scene)
                table_route, table_target = table_route_description(
                    scene, event, event_bytes
                )
                trigger_rows.append(
                    [
                        str(scene),
                        address(table_entry),
                        address(profile),
                        ",".join(map(str, profile_alias_groups[profile]))
                        if len(profile_alias_groups[profile]) > 1
                        else "",
                        address(collision_pointer_address),
                        address(collision_address),
                        ",".join(map(str, collision_alias_groups[collision_address]))
                        if len(collision_alias_groups[collision_address]) > 1
                        else "",
                        str(index),
                        address(record_address),
                        word(x0),
                        word(x1),
                        word(y0),
                        word(y1),
                        str(x0 // 0x20),
                        str(x1 // 0x20),
                        str(y0 // 0x20),
                        str(y1 // 0x20),
                        word(value),
                        f"0x{event:02X}",
                        address(event_pointer_address),
                        address(event_address) if event_address else "",
                        f"0x{table_byte:02X}" if table_byte is not None else "",
                        table_route,
                        table_target,
                        family,
                        handler_addresses,
                        native_functions,
                        physical_qa_route(scene, index, event),
                    ]
                )
        totals["collision_records"] += len(parsed)
        totals["scenes_with_event_table"] += bool(event_address)
        profiles.append(
            [
                str(scene),
                address(table_entry),
                address(profile),
                ",".join(map(str, profile_alias_groups[profile]))
                if len(profile_alias_groups[profile]) > 1
                else "",
                address(collision_pointer_address),
                address(collision_address),
                ",".join(map(str, collision_alias_groups[collision_address]))
                if len(collision_alias_groups[collision_address]) > 1
                else "",
                address(sentinel_address),
                str(len(parsed)),
                str(event_records),
                address(event_pointer_address),
                address(event_address) if event_address else "",
                ";".join(
                    f"{event:02X}:{count}"
                    for event, count in sorted(scene_counts.items())
                ),
                str(scene_dir.relative_to(args.assets)).replace("\\", "/"),
            ]
        )

    profile_path = args.docs / "field-map-scene-profiles.tsv"
    trigger_path = args.docs / "field-map-trigger-records.tsv"
    write_tsv(
        profile_path,
        [
            "scene_id",
            "scene_table_entry",
            "profile_address",
            "shared_profile_scene_ids",
            "collision_pointer_address",
            "collision_records_address",
            "shared_collision_scene_ids",
            "collision_sentinel_address",
            "collision_record_count",
            "event_record_count",
            "event_table_pointer_address",
            "event_table_address",
            "event_id_counts",
            "asset_directory",
        ],
        profiles,
    )
    write_tsv(
        trigger_path,
        [
            "scene_id",
            "scene_table_entry",
            "profile_address",
            "profile_alias_group_scene_ids",
            "collision_pointer_address",
            "collision_records_address",
            "collision_alias_group_scene_ids",
            "record_index_zero_based",
            "record_address",
            "x_start_raw",
            "x_end_raw",
            "y_start_raw",
            "y_end_raw",
            "x_start_grid_cell",
            "x_end_grid_cell",
            "y_start_grid_cell",
            "y_end_grid_cell",
            "raw_collision_value",
            "event_id_low_5_bits",
            "event_table_pointer_address",
            "event_table_address",
            "event_table_byte_at_event_id",
            "event_table_route_or_bypass",
            "event_table_target_class",
            "dispatcher_family",
            "rom_handler_addresses",
            "converted_cpp_functions_or_runtime_owners",
            "known_physical_qa_route",
        ],
        trigger_rows,
    )

    print(f"Verified {SCENE_COUNT} ROM scene profiles against extracted descriptors.")
    print(
        f"Verified {totals['collision_records']} collision records "
        f"({verified_collision_bytes:,} bytes) against ROM; "
        f"{totals['event_records']} high-bit event-bearing records profiled."
    )
    print(
        f"Those scene references cover {len(unique_collision_record_addresses)} "
        f"unique ROM collision-record addresses and "
        f"{len(unique_high_bit_record_addresses)} unique high-bit addresses."
    )
    print(f"Found {totals['scenes_with_event_table']} scene event tables.")
    print(f"Wrote {profile_path}")
    print(f"Wrote {trigger_path}")
    print("Event-bearing record counts by event ID:")
    for event, count in sorted(event_totals.items()):
        print(f"  0x{event:02X}: {count}")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (OSError, RuntimeError, ValueError, IndexError) as error:
        print(f"profile_field_map_triggers: {error}", file=sys.stderr)
        raise SystemExit(1)
