#!/usr/bin/env python3
"""Extract a ROM-backed map coverage matrix without changing runtime code.

The extractor reads the US ROM for scene profiles, event destinations,
elevator lists, cave links, and arrival records. Existing checked-in TSVs are
used only for named actor and weighted-event rows that were imported from the
same ROM; the TSV records retain their source offsets.
"""

from __future__ import annotations

import argparse
import os
import subprocess
import sys
import csv
import re
import struct
from collections import Counter
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
DEFAULT_ROM = Path("/workspace/shadowrun/rom.bin")
FALLBACK_ROM = ROOT / "decompilation/input/shadowrun_genesis_us.bin"
FIXTURE_DIR = ROOT / "decompilation/gameplay/tests_lua/fixtures/map_coverage"
DEFAULT_ROWS = FIXTURE_DIR / "rows.tsv"
DEFAULT_BASELINE = FIXTURE_DIR / "baseline_results.tsv"
SCENE_TABLE = 0x3232
SCENE_COUNT = 0x38
# ROM 0x3032 sets FFFC0E (cave mode) for these scenes.
CAVE_SCENES = frozenset((6, 7, 8, 9, *range(0x2C, 0x32)))
FIELDS = (
    "row_id",
    "scene",
    "category",
    "rom_addr",
    "trigger",
    "gate",
    "dest_scene",
    "arrival_xy",
    "arrival_mode",
    "port_ref",
)
CATEGORY_ORDER = {
    name: index
    for index, name in enumerate(
        ("edge", "door", "bexit", "elevator", "cave", "travel",
         "spawn", "revent", "trigger")
    )
}


def be16(data: bytes, address: int) -> int:
    return struct.unpack_from(">H", data, address)[0]


def be32(data: bytes, address: int) -> int:
    return struct.unpack_from(">I", data, address)[0]


def scene_mode(scene: int) -> int:
    if scene < 0x15:
        return 1
    if scene < 0x2C:
        return 2
    return 3


def read_tsv(path: Path) -> list[dict[str, str]]:
    if not path.exists():
        return []
    with path.open(encoding="utf-8", newline="") as handle:
        return list(csv.DictReader(handle, delimiter="\t"))


def source_line(path: Path, pattern: str) -> int | None:
    if not path.exists():
        return None
    for number, line in enumerate(path.read_text(encoding="utf-8").splitlines(), 1):
        if pattern in line:
            return number
    return None


def ref(path: Path, line: int | None) -> str:
    if line is None:
        return "MISSING"
    return f"{path.relative_to(ROOT).as_posix()}:{line}"


def make_row(
    row_id: str,
    scene: int | str,
    category: str,
    rom_addr: str,
    trigger: str,
    gate: str,
    dest_scene: int | str = "-",
    arrival_xy: str = "-",
    arrival_mode: int | str = "-",
    port_ref: str = "MISSING",
) -> dict[str, str]:
    return {
        "row_id": row_id,
        "scene": str(scene),
        "category": category,
        "rom_addr": rom_addr,
        "trigger": trigger,
        "gate": gate,
        "dest_scene": str(dest_scene),
        "arrival_xy": str(arrival_xy),
        "arrival_mode": str(arrival_mode),
        "port_ref": port_ref,
    }


def rom_collision_rows(rom: bytes) -> list[dict[str, object]]:
    rows: list[dict[str, object]] = []
    for scene in range(SCENE_COUNT):
        profile = be32(rom, SCENE_TABLE + scene * 4)
        records = be32(rom, profile + 0x26)
        events = be32(rom, profile + 0x4E)
        if not records or records + 10 > len(rom):
            continue
        address = records
        record_index = 0
        while address + 10 <= len(rom):
            x0, x1, y0, y1, raw_code = struct.unpack_from(">5H", rom, address)
            # The ROM sentinel is the sign bit in the first word (x0), as
            # recorded in field-map-scene-profiles.tsv; raw_code may be 0x1F
            # on ordinary unused cells and is not the list terminator.
            if x0 & 0x8000:
                break
            # Bit 7 marks a live event record. Unmarked 0x7F filler cells
            # otherwise alias selector 0x1F but are not menu/terminal events.
            selector = (raw_code & 0x1F) if raw_code & 0x80 else None
            destination = (
                rom[events + selector]
                if events and selector is not None else None
            )
            bounds = (x0 * 16, (x1 + 1) * 16 - 1,
                      y0 * 16, (y1 + 1) * 16 - 1)
            rows.append(
                {
                    "scene": scene,
                    "record": address,
                    "index": record_index,
                    "raw_code": raw_code,
                    "event": selector,
                    "destination": destination,
                    "bounds": bounds,
                    "profile": profile,
                    "event_table": events,
                }
            )
            address += 10
            record_index += 1
        else:
            raise ValueError(f"scene {scene} collision list has no ROM sentinel")
    return rows


def build_rows(rom_path: Path) -> list[dict[str, str]]:
    rom = rom_path.read_bytes()
    if len(rom) < 0x1EDB88:
        raise ValueError(f"ROM image is too short: {len(rom)} bytes")

    root = ROOT
    links_path = root / "decompilation/gameplay/scripts/data/world_links_data.lua"
    links_code_path = root / "decompilation/gameplay/scripts/world_links.lua"
    events_path = root / "decompilation/gameplay/scripts/field_events.lua"
    building_path = root / "decompilation/gameplay/scripts/building.lua"
    random_path = root / "decompilation/gameplay/scripts/field_random_events.lua"
    encounter_path = root / "decompilation/gameplay/scripts/field_encounters.lua"
    party_down_path = root / "decompilation/gameplay/scripts/party_down.lua"
    taxi_data_path = root / "decompilation/gameplay/scripts/data/taxi_destinations.lua"
    taxi_selector_path = root / "decompilation/gameplay/scripts/menu/screens/taxi_selector.lua"
    field_activities_path = root / "decompilation/gameplay/scripts/field_activities.lua"
    cave_data_path = root / "decompilation/gameplay/scripts/data/world_links_data.lua"
    link_lines: dict[int, int] = {}
    link_kinds: dict[int, str] = {}
    for number, line in enumerate(links_path.read_text(encoding="utf-8").splitlines(), 1):
        found = re.search(r"source record 0x([0-9A-Fa-f]{6})", line)
        if found:
            address = int(found.group(1), 16)
            link_lines[address] = number
            kind = re.search(r"transition = \{ kind = '([^']+)'", line)
            if kind:
                link_kinds[address] = kind.group(1)

    link_fn = source_line(links_code_path, "function WorldLinks.door_contact")
    edge_fn = source_line(links_code_path, "function WorldLinks.transition_at")
    return_fn = source_line(links_code_path, "function WorldLinks.building_service_exit_return")
    elevator_fn = source_line(building_path, "function Building.resolve_elevator_destination")
    event_fn = source_line(events_path, "local function")
    random_fn = source_line(random_path, "local RandomEvents")
    encounter_fn = source_line(encounter_path, "local Encounters")
    cave_fn = source_line(links_code_path, "function WorldLinks.cave_exit")
    cave_data_fn = source_line(cave_data_path, "M.cave_rooms = {")
    taxi_data_fn = source_line(taxi_data_path, "M.cursor_to_region =")
    taxi_selector_fn = source_line(taxi_selector_path, "local Taxi =")
    wake_fn = source_line(party_down_path, "function PartyDown.wake_arrival")
    phone_fn = source_line(field_activities_path, "function FieldActivities.vid_phone")

    rows: list[dict[str, str]] = []
    records = rom_collision_rows(rom)
    by_scene: dict[int, list[dict[str, object]]] = {
        scene: [item for item in records if item["scene"] == scene]
        for scene in range(SCENE_COUNT)
    }

    def record_ref(record: int, runtime: Path, runtime_line: int | None) -> str:
        data_line = link_lines.get(record)
        return ";".join(
            value for value in (
                ref(links_path, data_line),
                ref(runtime, runtime_line),
            ) if value != "MISSING"
        ) or "MISSING"

    # Collision rectangles whose ROM event is 0x10 are the physical edges.
    # 0x3168 runs only in outdoor mode (FFDF40=0, 0x0992). Scene 0 picks
    # scene 3 or 5 by FFDDC6 against 0x1400; any other scene returns to 0.
    # Each branch also loads an arrival block (camera pair, then three party
    # points) into FFE114: 0x3212 (to 5), 0x3222 (to 3), 0x31F2 (from 5),
    # 0x3202 (from any other scene). Only event-0x10 records in scenes 0, 3
    # and 5 exist, so no per-scene rule rows are emitted, and a scene-0 rect
    # yields only the branch its own position can reach (a rect wholly west
    # of 0x1400 can never satisfy FFDDC6>=0x1400 and vice versa).
    for item in records:
        scene = int(item["scene"])
        if item["event"] is None:
            continue
        event = int(item["event"])
        if event != 0x10:
            continue
        x0, x1, y0, y1 = item["bounds"]  # type: ignore[misc]
        address = int(item["record"])
        if scene == 0:
            if x0 >= 0x1400:
                branches = (("camX>=0x1400", 5, 0x3212),)
            elif x1 < 0x1400:
                branches = (("camX<0x1400", 3, 0x3222),)
            else:
                branches = (("camX<0x1400", 3, 0x3222),
                            ("camX>=0x1400", 5, 0x3212))
        elif scene == 5:
            branches = ((f"scene={scene}", 0, 0x31F2),)
        else:
            branches = ((f"scene={scene}", 0, 0x3202),)
        for branch, destination, arrival in branches:
            branch_id = branch.replace("<", "lt").replace(">", "gt").replace("=", "eq")
            lead = f"0x{be16(rom, arrival + 4):04X},0x{be16(rom, arrival + 6):04X}"
            rows.append(make_row(
                f"edge-s{scene:02x}-r{address:06x}-{branch_id}", scene, "edge",
                f"0x{address:06X};arrival=0x{arrival:06X};consume=0x0B7B8>0x1651E>0x1F5CA>0x03168",
                f"rect:x={x0}..{x1},y={y0}..{y1};event=0x10;{branch}",
                "FFDF40=0", destination, lead, 0,
                record_ref(address, links_code_path, edge_fn),
            ))

    direct_doors: list[dict[str, object]] = []
    for item in records:
        scene = int(item["scene"])
        if item["event"] is None:
            continue
        event = int(item["event"])
        destination = item["destination"]
        address = int(item["record"])
        # Event 0 is a return or local event. 0x1D is the floor selector;
        # 0x1F is the field terminal. Event 0x10 was emitted above.
        if event in (0, 0x10, 0x1D, 0x1F):
            continue
        # 0x0A12-0x0A28: with FFFC0E set (0x3032: scenes 6-9, 0x2C-0x31)
        # events 8..0x0B go to the cave handler 0x2CE0, never 0x167AE; they
        # are cave rows. 0x09FA skips scene 4 event 6 entirely.
        if scene in CAVE_SCENES and event >= 8:
            continue
        if scene == 4 and event == 6:
            continue
        # The importer labels direct scene changes separately from contextual
        # handlers and same-map services. Those latter rows stay category 9.
        if link_kinds.get(address) != "scene_change":
            continue
        if destination is None or not 0 <= int(destination) < SCENE_COUNT:
            continue
        if int(destination) == scene:
            continue
        direct_doors.append(item)
        x0, x1, y0, y1 = item["bounds"]  # type: ignore[misc]
        dest = int(destination)
        rows.append(make_row(
            f"door-s{scene:02x}-r{address:06x}", scene, "door",
            f"0x{address:06X};table=0x{int(item['event_table']) + event:06X};consume=0x0B7B8>0x1651E>0x1F5CA>0x167AE",
            f"rect:x={x0}..{x1},y={y0}..{y1};event=0x{event:02X};FFDE14[{event}]=0x{dest:02X}",
            "FFDF40=0",
            dest, "scene-entry ROM position not resolved by this record",
            scene_mode(dest),
            record_ref(address, links_code_path, link_fn),
        ))

    # Pair each direct entrance with the event-0 return rectangle in its
    # destination scene. The saved FFDE26 value is the arrival point.
    for door in direct_doors:
        source = int(door["scene"])
        interior = int(door["destination"])
        exits = [item for item in by_scene[interior]
                 if item["event"] is not None and int(item["event"]) == 0]
        for exit_item in exits:
            exit_address = int(exit_item["record"])
            entry_address = int(door["record"])
            x0, x1, y0, y1 = exit_item["bounds"]  # type: ignore[misc]
            rows.append(make_row(
                f"bexit-s{interior:02x}-entry{entry_address:06x}-r{exit_address:06x}",
                interior, "bexit",
                f"entry=0x{entry_address:06X};exit=0x{exit_address:06X};consume=0x0A98>0x02F32",
                f"rect:x={x0}..{x1},y={y0}..{y1};event=0x00;restore=FFDE26",
                f"FFDF40={scene_mode(interior)}",
                source, "FFDE26 (saved source door point)", 0,
                record_ref(exit_address, links_code_path, return_fn),
            ))

    # Elevator child maps are read through profile+0x58 -> resource pointer ->
    # list header, as 0x167AE/0x16A8E do. Each floor is a separate selectable
    # result reached through an event-0x1D collision rectangle.
    for item in records:
        if item["event"] is None or int(item["event"]) != 0x1D:
            continue
        scene = int(item["scene"])
        profile = int(item["profile"])
        resource = be32(rom, profile + 0x58)
        if not resource or resource + 4 > len(rom):
            continue
        group = be32(rom, resource)
        if not group or group + 2 > len(rom):
            continue
        max_floor = be16(rom, group)
        if max_floor > 5 or group + 2 * (max_floor + 2) > len(rom):
            continue
        children = [be16(rom, group + 2 * (index + 1))
                    for index in range(max_floor + 1)]
        if any(child >= SCENE_COUNT for child in children):
            continue
        address = int(item["record"])
        x0, x1, y0, y1 = item["bounds"]  # type: ignore[misc]
        for floor, destination in enumerate(children):
            # Choosing the current floor reloads nothing; not an exit.
            if destination == scene:
                continue
            rows.append(make_row(
                f"elevator-s{scene:02x}-r{address:06x}-floor{floor}",
                scene, "elevator",
                f"0x{address:06X};group=0x{group:06X};consume=0x0167AE>0x58DE0>0x16A8E",
                f"rect:x={x0}..{x1},y={y0}..{y1};event=0x1D;floor={floor}",
                f"FFDF40={scene_mode(scene)}",
                destination, "scene-entry ROM position not resolved by this record",
                scene_mode(scene),
                record_ref(address, building_path, elevator_fn),
            ))

    # Cave records: 24 room rows, four direction slots. ROM byte tables at
    # 0x2E42/0x2E72/0x2ED2 join the selected room to the scene and arrival
    # coordinates at 0x2D92.
    directions = ("north", "east", "south", "west")
    for room in range(24):
        source_scene = be16(rom, 0x2E42 + room * 2)
        for direction_index, direction in enumerate(directions):
            slot = 0x2E72 + room * 4 + direction_index
            next_room = rom[slot]
            arrival_slot = rom[0x2ED2 + room * 4 + direction_index]
            if next_room >= 24:
                continue
            destination = be16(rom, 0x2E42 + next_room * 2)
            arrival_row = destination - 6 if destination < 0x2C else destination - 0x2C + 4
            xy_address = 0x2D92 + arrival_row * 16 + arrival_slot * 4
            x, y = be16(rom, xy_address), be16(rom, xy_address + 2)
            event = 8 + direction_index
            matches = [
                trigger for trigger in by_scene[source_scene]
                if trigger["event"] is not None and int(trigger["event"]) == event
            ]
            if not matches:
                matches = [None]
            # A direction with no event-(8+dir) rect in the room's scene
            # cannot be taken; it is table padding, not an exit.
            if matches == [None]:
                continue
            for trigger in matches:
                if trigger is None:
                    trigger_text = (
                        f"room={room};direction={direction};event=0x{event:02X};"
                        "no collision rectangle in this scene profile"
                    )
                    trigger_address = 0
                else:
                    x0, x1, y0, y1 = trigger["bounds"]  # type: ignore[misc]
                    trigger_address = int(trigger["record"])
                    trigger_text = (
                        f"rect:x={x0}..{x1},y={y0}..{y1};event=0x{event:02X};"
                        f"room={room};direction={direction}"
                    )
                trigger_address_text = (
                    f"0x{trigger_address:06X};" if trigger_address else ""
                )
                rows.append(make_row(
                    f"cave-room{room:02x}-{direction}-r{trigger_address:06x}",
                    source_scene, "cave",
                    f"{trigger_address_text}entry=0x{slot:06X};"
                    f"next_room=0x{slot:06X};scene_entry=0x{0x2E42 + room * 2:06X};"
                    f"arrival=0x{xy_address:06X};"
                    "consume=0x02CE0",
                    trigger_text,
                    f"FFFC10={room};FFFC0E!=0;FFFBF9=0x{event:02X}",
                    destination, f"0x{x:04X},0x{y:04X}", 3,
                    f"{ref(cave_data_path, cave_data_fn)};{ref(links_code_path, cave_fn)}",
                ))

    # Death recovery has two source continuation records. The lead positions
    # are the second coordinate pair in each ROM block; the first pair is the
    # camera anchor.
    for scene, address, destination, x, y in (
        (0, 0x2AEA, 0, 0x6900, 0x3A80),
        (2, 0x2B24, 2, 0x6F00, 0x2780),
    ):
        rows.append(make_row(
            f"travel-death-s{scene:02x}-{address:06x}", scene, "travel",
            f"0x{address:06X};wake=0x02B78;consume=0x02A8E/0x02B0E",
            "party-down;wake-continuation",
            "death-recovery",
            destination, f"0x{x:04X},0x{y:04X}", 0,
            ref(party_down_path, wake_fn),
        ))

    # Taxi region and cursor tables: 0xD4456 maps the current field region to
    # a cab-map cursor, 0xD445D maps a chosen cursor to a destination region,
    # and 0x20912 publishes that region before 0x136DE loads it.
    for source_region in range(7):
        current_cursor = rom[0xD4456 + source_region]
        for cursor in range(7):
            destination = rom[0xD445D + cursor]
            if current_cursor >= 7 or destination >= SCENE_COUNT:
                continue
            rows.append(make_row(
                f"travel-taxi-r{source_region}-cursor{cursor}",
                source_region, "travel",
                f"0x0D4456+{source_region};0x0D445D+{cursor};consume=0x020912>0x0136B6>0x0136DE",
                f"Call Taxi menu;current_cursor={current_cursor};input_cursor={cursor}",
                "field terminal and taxi confirmation",
                destination, "scene-entry ROM position not resolved by this record",
                0,
                f"{ref(taxi_data_path, taxi_data_fn)};{ref(taxi_selector_path, taxi_selector_fn)}",
            ))

    # Fixed paid/NPC travel continuations. D0 is selected dynamically at
    # 0x21E84 and 0x24DB6; the branch immediately before 0x229F2 sets D0=6.
    for address, scene, destination, condition in (
        (0x21E84, "-", "D0", "paid travel confirmation"),
        (0x229F2, "-", 6, "nuyen debit accepted;D0=6"),
        (0x24DB6, "-", "D0", "NPC travel confirmation"),
    ):
        rows.append(make_row(
            f"travel-paid-{address:06x}", scene, "travel",
            f"0x{address:06X};consume=0x0136DE",
            condition, "route-specific fare/story gate",
            destination, "scene-entry ROM position not resolved by this record", 0,
            ref(party_down_path, None),
        ))

    # Terminal records are input-only entry points for Call Taxi/Vid-Phone.
    # Their menu result depends on controller choice, so no fixed destination
    # scene is asserted here.
    for item in records:
        if item["event"] is None or int(item["event"]) != 0x1F:
            continue
        scene = int(item["scene"])
        address = int(item["record"])
        x0, x1, y0, y1 = item["bounds"]  # type: ignore[misc]
        rows.append(make_row(
            f"travel-terminal-s{scene:02x}-r{address:06x}", scene, "travel",
            f"0x{address:06X};menu=0x0DB26;taxi=0xD4456/0xD445D;vid-phone=0x0E4C8",
            f"rect:x={x0}..{x1},y={y0}..{y1};event=0x1F;menu=Call Taxi/Vid-Phone/Cyberspace/Exit",
            "field-terminal",
            "menu-selected", "preserve source position unless a map route is selected",
            "selected route",
            record_ref(address, events_path, event_fn),
        ))
        rows.append(make_row(
            f"travel-vidphone-s{scene:02x}-r{address:06x}",
            scene, "travel",
            f"0x{address:06X};field-terminal=0x0DB26;vid-phone=0x0E4C8;return=0x22CE8",
            f"rect:x={x0}..{x1},y={y0}..{y1};event=0x1F;menu=Vid-Phone;call-and-return",
            "field terminal;contact menu",
            scene, "preserve source position", "preserve source FFDF40",
            ref(field_activities_path, phone_fn),
        ))

    # Static spawn lists and weighted event cells are already offline imports
    # from the same ROM. Their row fields retain the precise source offsets,
    # scene gate, and actor/event condition.
    defaults_path = root / "decompilation/assets/field/entities/default-actors.tsv"
    conditional_path = root / "decompilation/assets/field/entities/conditional-actor-candidates.tsv"
    for index, actor in enumerate(read_tsv(defaults_path)):
        scene = int(actor["scene"])
        rows.append(make_row(
            f"spawn-default-s{scene:02x}-n{actor['source_order']}",
            scene, "spawn",
            f"{actor['list_offset']};position={actor['position_offset']};"
            f"template={actor['template_offset']};consume=0x03074>0x03084",
            f"scene-entry;actor={actor['actor_name']};xy={actor['world_x']},{actor['world_y']}",
            f"source_spawn_state_flags=0x{int(actor['source_spawn_state_flags']):02X}",
            scene, f"{actor['world_x']},{actor['world_y']}", "-",
            f"{defaults_path.relative_to(root).as_posix()}:{index + 2}",
        ))
    for index, actor in enumerate(read_tsv(conditional_path)):
        scene = int(actor["scene"])
        rows.append(make_row(
            f"spawn-conditional-s{scene:02x}-choice{actor['choice']}",
            scene, "spawn",
            f"{actor['list_offset']};position={actor['position_offset']};"
            f"template={actor['template_offset']};consume=0x0A800>0x5BA98",
            f"scene-entry;actor={actor['actor_name']};xy={actor['world_x']},{actor['world_y']};choice={actor['choice']}",
            actor["activation_gate"],
            scene, f"{actor['world_x']},{actor['world_y']}", "-",
            f"{conditional_path.relative_to(root).as_posix()}:{index + 2}",
        ))
    for asset_name in ("world-activity-actors.tsv",
                       "dynamic-world-activity-actors.tsv"):
        actors_path = root / "decompilation/assets/field/entities" / asset_name
        for index, actor in enumerate(read_tsv(actors_path)):
            scene = int(actor["scene"])
            gate = actor.get("activation_rule") or actor.get("placement_rule") or "ROM selector gate"
            list_id = actor.get("list_id") or actor.get("context_group") or "-"
            source_offset = (
                actor.get("source_record_offset")
                or actor.get("sequence_list_offset")
                or actor.get("list_offset")
                or actor.get("selection_cell_offset")
                or "-"
            )
            position_offset = actor.get("position_offset", "-")
            rows.append(make_row(
                f"spawn-activity-{asset_name.split('.')[0]}-s{scene:02x}-n{index}",
                scene, "spawn",
                f"{source_offset};position={position_offset};"
                "consume=0x1E166>0x59986",
                f"world-activity list={list_id};actor={actor.get('actor_name', '-')};"
                f"xy={actor.get('world_x', 'dynamic')},{actor.get('world_y', 'dynamic')}",
                gate, scene,
                f"{actor.get('world_x', 'dynamic')},{actor.get('world_y', 'dynamic')}",
                "-",
                f"{actors_path.relative_to(root).as_posix()}:{index + 2};"
                f"{ref(random_path, random_fn)};{ref(encounter_path, encounter_fn)}",
            ))

    activity_path = root / "decompilation/assets/field/world-activity-events.tsv"
    activity_outcomes_path = root / "decompilation/assets/field/world-activity-outcomes.tsv"
    for index, outcome in enumerate(read_tsv(activity_outcomes_path)):
        scene = int(outcome["scene"])
        event_id = outcome.get("handler_offset") or outcome.get("outcome_kind")
        rows.append(make_row(
            f"revent-s{scene:02x}-sum{outcome['random_sum']}-cell{index}",
            scene, "revent",
            f"{outcome['selection_cell_offset']};pointer={outcome['selection_pointer_offset']};"
            f"consume=0x1E166>0x59986",
            f"weighted-cell;event_id={event_id};random_sum={outcome['random_sum']};"
            f"weight={outcome['roll_pair_count']};scene={scene}",
            outcome["activation"],
            "-", "dynamic result", "-",
            f"{activity_outcomes_path.relative_to(root).as_posix()}:{index + 2};"
            f"{ref(random_path, random_fn)};{ref(encounter_path, encounter_fn)}",
        ))

    # The 779 contextual_transition and 16 unresolved_destination records are
    # deliberately static-only in this phase. Same-map services are included
    # here too because they do not select a fixed map destination.
    for item in records:
        address = int(item["record"])
        kind = link_kinds.get(address)
        if kind not in ("contextual_transition", "unresolved_destination",
                        "same_map_service"):
            continue
        scene = int(item["scene"])
        event = item["event"]
        x0, x1, y0, y1 = item["bounds"]  # type: ignore[misc]
        event_text = f"0x{int(event):02X}" if event is not None else "no event"
        rows.append(make_row(
            f"trigger-{kind}-s{scene:02x}-r{address:06x}",
            scene, "trigger",
            f"0x{address:06X};consume=0x0B7B8>0x1651E>0x1F5CA",
            f"rect:x={x0}..{x1},y={y0}..{y1};event={event_text};kind={kind}",
            f"{kind};runtime gate not resolved by this static extractor",
            "contextual" if kind == "contextual_transition" else (
                "unresolved" if kind == "unresolved_destination" else "same scene"
            ),
            "-", "-", record_ref(address, links_code_path, link_fn),
        ))

    # All other collision handlers and one entry-dispatch row per scene remain
    # static-presence checks in this run.
    for item in records:
        scene = int(item["scene"])
        if item["event"] is None:
            continue
        event = int(item["event"])
        if event in (0x10, 0x1D, 0x1F):
            continue
        address = int(item["record"])
        if address in link_kinds:
            continue
        if any(row["category"] in ("door", "bexit") and
               f"0x{address:06X}" in row["rom_addr"] for row in rows):
            continue
        x0, x1, y0, y1 = item["bounds"]  # type: ignore[misc]
        rows.append(make_row(
            f"trigger-s{scene:02x}-r{address:06x}", scene, "trigger",
            f"0x{address:06X};consume=0x0B7B8>0x1651E>0x1F5CA",
            f"rect:x={x0}..{x1},y={y0}..{y1};event=0x{event:02X}",
            "event-specific ROM gate; see handler",
            item["destination"] if item["destination"] is not None else "-",
            "-", "-", record_ref(address, events_path, event_fn),
        ))
    for scene in range(SCENE_COUNT):
        profile_entry = SCENE_TABLE + scene * 4
        rows.append(make_row(
            f"trigger-entry-s{scene:02x}", scene, "trigger",
            f"0x{profile_entry:06X};consume=0x03074>0x03084",
            "scene-entry dispatcher",
            "scene-specific ROM entry gate",
            scene, "scene-entry position", "-",
            f"{ref(events_path, event_fn)};{ref(links_code_path, link_fn)}",
        ))

    rows.sort(key=lambda row: (
        CATEGORY_ORDER[row["category"]],
        int(row["scene"]) if row["scene"].isdigit() else -1,
        row["row_id"],
    ))
    return rows


def self_test(rows: list[dict[str, str]]) -> None:
    by_id = {row["row_id"]: row for row in rows}
    door = by_id["door-s00-r0d57f0"]
    assert door["trigger"] == (
        "rect:x=27136..27647,y=10240..10751;event=0x01;FFDE14[1]=0x0C"
    ), door
    assert door["scene"] == "0" and door["dest_scene"] == "12"
    assert door["arrival_mode"] == "1"

    death = by_id["travel-death-s00-002aea"]
    assert death["scene"] == "0"
    assert death["arrival_xy"] == "0x6900,0x3A80"
    assert "0x002AEA" in death["rom_addr"]

    edge = by_id["edge-s00-r0d582c-camXgteq0x1400"]
    assert edge["scene"] == "0" and edge["dest_scene"] == "5"
    assert edge["arrival_xy"] == "0x0600,0x3A00", edge
    assert "edge-s00-r0d5804-camXgteq0x1400" not in by_id
    assert by_id["edge-s00-r0d5804-camXlt0x1400"]["arrival_xy"] == "0x7880,0x2B00"
    assert not any(row_id.startswith("edge-rule-") for row_id in by_id)
    assert "door-s07-r1b459c" not in by_id
    assert "cave-room01-north-r1b459c" in by_id
    assert "elevator-s1a-r1d9d6a-floor0" not in by_id


def write_rows(path: Path, rows: list[dict[str, str]]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w", encoding="utf-8", newline="") as handle:
        writer = csv.DictWriter(handle, fieldnames=FIELDS, delimiter="\t",
                                lineterminator="\n")
        writer.writeheader()
        writer.writerows(rows)


def write_report(rows_path: Path, results_path: Path, report_path: Path) -> None:
    with rows_path.open(encoding="utf-8", newline="") as handle:
        rows = list(csv.DictReader(handle, delimiter="\t"))
    with results_path.open(encoding="utf-8", newline="") as handle:
        results = {
            row["row_id"]: row
            for row in csv.DictReader(handle, delimiter="\t")
        }

    category_counts: dict[str, Counter[str]] = {
        category: Counter() for category in CATEGORY_ORDER
    }
    scene_counts: dict[int, Counter[str]] = {
        scene: Counter() for scene in range(SCENE_COUNT)
    }
    unresolved: list[tuple[dict[str, str], dict[str, str]]] = []
    for row in rows:
        result = results.get(row["row_id"], {})
        status = result.get("status", "MISSING") or "MISSING"
        category_counts[row["category"]][status] += 1
        try:
            scene = int(row["scene"])
        except ValueError:
            scene = -1
        if scene in scene_counts:
            scene_counts[scene][status] += 1
        if status in ("WRONG", "MISSING"):
            unresolved.append((row, result))

    def priority(entry: tuple[dict[str, str], dict[str, str]]) -> tuple[int, int, str]:
        row, result = entry
        status_rank = 0 if result.get("status") == "WRONG" else 1
        category_rank = CATEGORY_ORDER[row["category"]]
        return status_rank, category_rank, row["row_id"]

    selected: list[tuple[dict[str, str], dict[str, str]]] = []
    scene_order = [0, 2, *range(7, SCENE_COUNT), 1, 3, 4, 5, 6]
    for scene in scene_order:
        scene_rows = [entry for entry in unresolved
                      if entry[0]["scene"].isdigit() and
                      int(entry[0]["scene"]) == scene]
        scene_rows.sort(key=priority)
        take = 8 if scene in (0, 2) else 25 - len(selected)
        selected.extend(scene_rows[:max(0, min(take, 25 - len(selected)))])
        if len(selected) >= 25:
            break
    if len(selected) < 25:
        global_rows = [entry for entry in unresolved
                       if not entry[0]["scene"].isdigit()]
        global_rows.sort(key=priority)
        selected.extend(global_rows[:25 - len(selected)])
    lines = [
        "# ROM-to-runtime map coverage",
        "",
        "Results come from `baseline_results.tsv`. Categories 7–9 are static presence checks only. Most MISSING/WRONG rows are known harness gaps (approach side, elevator selector, cave room seeding, taxi region mapping, exit-offset expectation), not confirmed port bugs; see the mapharness briefs.",
        "",
        "## Totals by category and status",
        "",
        "| Category | PASS | WRONG | MISSING | PRESENT-UNTESTED | Total |",
        "|---|---:|---:|---:|---:|---:|",
    ]
    overall = Counter()
    for category in CATEGORY_ORDER:
        counts = category_counts[category]
        total = sum(counts.values())
        overall.update(counts)
        lines.append(
            f"| {category} | {counts['PASS']} | {counts['WRONG']} | "
            f"{counts['MISSING']} | {counts['PRESENT-UNTESTED']} | {total} |"
        )
    lines.append(
        f"| **Total** | **{overall['PASS']}** | **{overall['WRONG']}** | "
        f"**{overall['MISSING']}** | **{overall['PRESENT-UNTESTED']}** | "
        f"**{sum(overall.values())}** |"
    )
    lines.extend([
        "",
        "## Per-scene status",
        "",
        "| Scene | Name | PASS | WRONG | MISSING | PRESENT-UNTESTED |",
        "|---:|---|---:|---:|---:|---:|",
    ])
    for scene in range(SCENE_COUNT):
        counts = scene_counts[scene]
        lines.append(
            f"| {scene:02X} | Scene {scene:02X} | {counts['PASS']} | "
            f"{counts['WRONG']} | {counts['MISSING']} | "
            f"{counts['PRESENT-UNTESTED']} |"
        )
    lines.extend(["", "## 25 highest-priority WRONG or MISSING rows", ""])
    if selected:
        lines.extend([
            "| Row | Scene | Category | Status | Observed | Diff |",
            "|---|---:|---|---|---|---|",
        ])
        for row, result in selected:
            values = (
                f"event={result.get('observed_event', '-')}",
                f"dest={result.get('dest_scene', '-')}",
                f"xy={result.get('arrival_xy', '-')}",
                f"mode={result.get('arrival_mode', '-')}",
            )
            observed = ", ".join(value for value in values if not value.endswith("=-"))
            diff = (result.get("diff", "") or "").replace("|", "/")
            lines.append(
                f"| `{row['row_id']}` | {row['scene']} | {row['category']} | "
                f"{result.get('status', 'MISSING')} | {observed or '-'} | {diff} |"
            )
    else:
        lines.append("All rows are PASS or PRESENT-UNTESTED.")
    report_path.parent.mkdir(parents=True, exist_ok=True)
    report_path.write_text("\n".join(lines) + "\n", encoding="utf-8")


RUNTIME_CATEGORIES = ("edge", "door", "bexit", "elevator", "cave", "travel")
# Only the semantic columns are compared for drift; port_ref holds source line
# numbers that move whenever unrelated Lua is edited.
DRIFT_FIELDS = tuple(name for name in FIELDS if name != "port_ref")


def read_results(path: Path) -> dict[str, dict[str, str]]:
    return {row["row_id"]: row for row in read_tsv(path)}


def check_drift(rows: list[dict[str, str]], checked_in: Path) -> list[str]:
    """Return differences between fresh ROM rows and the checked-in rows."""
    fresh = {row["row_id"]: row for row in rows}
    stored = {row["row_id"]: row for row in read_tsv(checked_in)}
    problems = []
    for row_id in sorted(set(fresh) | set(stored)):
        if row_id not in stored:
            problems.append(f"new row not checked in: {row_id}")
        elif row_id not in fresh:
            problems.append(f"checked-in row no longer extracted: {row_id}")
        else:
            for name in DRIFT_FIELDS:
                if fresh[row_id][name] != stored[row_id][name]:
                    problems.append(
                        f"{row_id}: {name} {stored[row_id][name]!r} -> "
                        f"{fresh[row_id][name]!r}")
    return problems


def compare_results(baseline: dict[str, dict[str, str]],
                    current: dict[str, dict[str, str]],
                    row_ids: list[str]) -> tuple[list[str], list[str]]:
    """Regressions fail; improvements are reported so the baseline can move.

    A regression is a row the baseline records as PASS that is now anything
    else (or absent). WRONG/MISSING rows that stay non-PASS never fail, so
    known gaps do not break the suite while fixes can flip rows to PASS.
    """
    regressions: list[str] = []
    improvements: list[str] = []
    for row_id in row_ids:
        before = baseline.get(row_id, {}).get("status", "MISSING")
        after = current.get(row_id, {}).get("status", "ABSENT")
        if before == "PASS" and after != "PASS":
            detail = current.get(row_id, {}).get("diff", "no result row")
            regressions.append(f"{row_id}: PASS -> {after} ({detail})")
        elif before != "PASS" and after == "PASS":
            improvements.append(f"{row_id}: {before} -> PASS")
    return regressions, improvements


def run_qa(args: argparse.Namespace, rows: list[dict[str, str]]) -> int:
    baseline = read_results(args.baseline)
    if not baseline:
        print(f"map coverage: baseline {args.baseline} is empty", file=sys.stderr)
        return 2
    selected = [row for row in rows if row["category"] in RUNTIME_CATEGORIES]
    if args.only_baseline_pass:
        selected = [row for row in selected
                    if baseline.get(row["row_id"], {}).get("status") == "PASS"]
    if args.categories:
        wanted = set(args.categories.split(","))
        selected = [row for row in selected if row["category"] in wanted]
    if args.match:
        selected = [row for row in selected if re.search(args.match, row["row_id"])]
    if not selected:
        print("map coverage: no rows selected", file=sys.stderr)
        return 2
    args.out_dir.mkdir(parents=True, exist_ok=True)
    rows_path = args.out_dir / "rows.tsv"
    results_path = args.out_dir / "results.tsv"
    write_rows(rows_path, selected)
    env = dict(os.environ)
    env.setdefault("QT_QPA_PLATFORM", "offscreen")
    env.setdefault("SRX_HEADLESS", "1")
    env.setdefault("SDL_AUDIODRIVER", "dummy")
    env.setdefault("ALSOFT_DRIVERS", "null")
    command = [str(args.run_qa), "--assets", str(args.assets),
               "--map-coverage", str(rows_path), str(results_path)]
    print(f"map coverage: running {len(selected)} rows", flush=True)
    completed = subprocess.run(command, env=env, check=False)
    if completed.returncode != 0:
        print(f"map coverage: harness exited {completed.returncode}", file=sys.stderr)
        return 1
    current = read_results(results_path)
    regressions, improvements = compare_results(
        baseline, current, [row["row_id"] for row in selected])
    counts = Counter(current.get(row["row_id"], {}).get("status", "ABSENT")
                     for row in selected)
    print("map coverage: " + ", ".join(f"{k}={v}" for k, v in sorted(counts.items())))
    for line in improvements:
        print(f"IMPROVED {line}")
    for line in regressions:
        print(f"REGRESSED {line}", file=sys.stderr)
    if improvements:
        print(f"map coverage: {len(improvements)} rows now PASS; refresh "
              f"{args.baseline} with these results to lock them in")
    print(f"map coverage: results in {results_path}")
    return 1 if regressions else 0


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--rom", type=Path, default=DEFAULT_ROM)
    parser.add_argument("--rows", type=Path, default=DEFAULT_ROWS)
    parser.add_argument("--self-test", action="store_true")
    parser.add_argument("--report-results", type=Path)
    parser.add_argument("--report", type=Path)
    parser.add_argument("--check-drift", action="store_true",
                        help="fail if --rows differs from a fresh ROM extraction")
    parser.add_argument("--run-qa", type=Path,
                        help="QA executable; run the harness and compare to --baseline")
    parser.add_argument("--assets", type=Path,
                        default=ROOT / "decompilation/assets")
    parser.add_argument("--baseline", type=Path, default=DEFAULT_BASELINE)
    parser.add_argument("--out-dir", type=Path, default=ROOT / "out/map_coverage")
    parser.add_argument("--only-baseline-pass", action="store_true")
    parser.add_argument("--categories")
    parser.add_argument("--match")
    args = parser.parse_args()
    rom_path = args.rom if args.rom.exists() else FALLBACK_ROM
    rows = build_rows(rom_path)
    if args.self_test:
        self_test(rows)
        print("map_coverage_self_test=PASS")
    if args.check_drift:
        problems = check_drift(rows, args.rows)
        for problem in problems[:50]:
            print(f"DRIFT {problem}", file=sys.stderr)
        if problems:
            print(f"map coverage: {len(problems)} drift lines; regenerate with "
                  f"tools/map_coverage.py --rows {args.rows}", file=sys.stderr)
            return 1
        print("map_coverage_drift=NONE")
        return 0
    if args.run_qa is not None:
        return run_qa(args, rows)
    write_rows(args.rows, rows)
    if args.report_results is not None and args.report is not None:
        write_report(args.rows, args.report_results, args.report)
    counts = Counter(row["category"] for row in rows)
    print(f"rom={rom_path}")
    print(f"rows={len(rows)} scenes={SCENE_COUNT}")
    print("categories=" + ",".join(
        f"{name}:{counts[name]}" for name in CATEGORY_ORDER
    ))
    print(f"output={args.rows}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
