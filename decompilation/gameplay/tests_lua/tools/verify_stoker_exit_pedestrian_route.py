#!/usr/bin/env python3
"""Replay the cold-boot #34c controller route and check it against GPGX RAM."""

from __future__ import annotations

import json
import re
import struct
import subprocess
import sys
from pathlib import Path


QT_KEYS = {
    "START": 32,
    "A": 90,
    "B": 88,
    "LEFT": 16777234,
    "UP": 16777235,
    "DOWN": 16777237,
}


def fail(message: str) -> None:
    raise SystemExit(f"#34c route QA: {message}")


def input_log_from_route(route_path: Path, output_path: Path) -> int:
    route_text = route_path.read_text(encoding="utf-8")
    frame_match = re.search(r"frame_count\s*=\s*(\d+)", route_text)
    if frame_match is None:
        fail("route.lua has no frame_count")
    frame_count = int(frame_match.group(1))
    action_pattern = re.compile(
        r"\{\s*frame\s*=\s*(\d+)\s*,\s*pad\s*=\s*E\.Pad\.([A-Z_]+)\s*,"
        r"\s*pressed\s*=\s*(true|false)\s*\}"
    )
    actions = [
        (int(frame), QT_KEYS[pad], pressed == "true")
        for frame, pad, pressed in action_pattern.findall(route_text)
        if pad in QT_KEYS
    ]
    if not actions or max(event[0] for event in actions) >= frame_count:
        fail("route.lua has no supported input actions or an action past its end")
    output_path.write_bytes(
        b"SRXKEY1\0"
        + struct.pack("<QQ", frame_count, len(actions))
        + b"".join(
            struct.pack("<QII", frame, key, 0) + bytes((pressed,))
            for frame, key, pressed in actions
        )
    )
    return frame_count


def read_gpgx_frames(path: Path) -> dict[int, dict[str, str]]:
    lines = [line for line in path.read_text(encoding="utf-8").splitlines()
             if line and not line.startswith("#")]
    headers = lines[0].split("\t")
    return {
        int(values[0]): dict(zip(headers, values))
        for values in (line.split("\t") for line in lines[1:])
    }


def parse_source_slots(row: dict[str, str]) -> dict[int, dict[str, int]]:
    result: dict[int, dict[str, int]] = {}
    for record in row["ped_slots_3_5"].split(";"):
        slot_text, content = record.split(":", 1)
        values = content.split(",")
        if int(values[2].split("=", 1)[1], 16) == 0:
            continue
        result[int(slot_text)] = {
            "x": int(values[0], 16),
            "y": int(values[1], 16),
            "entity_flags": int(values[2].split("=", 1)[1], 16),
            "spawn_flags": int(values[3].split("=", 1)[1], 16),
        }
    return result


def port_pedestrians(row: dict[str, object]) -> dict[int, dict[str, int]]:
    result: dict[int, dict[str, int]] = {}
    for key, value in row.items():
        if not key.startswith("actor.") or not key.endswith(".profile"):
            continue
        if value != "pedestrian_k0b_none00":
            continue
        prefix = key[:-len(".profile")]
        actor = {name[len(prefix) + 1:]: field
                 for name, field in row.items()
                 if name.startswith(prefix + ".")}
        slot = actor.get("record_slot")
        if not isinstance(slot, (int, float)):
            continue
        result[int(slot)] = {
            "x": int(actor["x"]),
            "y": int(actor["y"]),
            "entity_flags": int(actor["entity_flags"]),
            "spawn_flags": int(actor["special_flags"]),
        }
    return result


def first_movement_step(rows, slot: int, actors, frame_key: str):
    previous_position = None
    for row in rows:
        actor = actors(row).get(slot)
        if actor is None:
            previous_position = None
            continue
        position = (actor["x"], actor["y"])
        if previous_position is not None and position != previous_position:
            return {
                "frame": int(row[frame_key]),
                "dx": position[0] - previous_position[0],
                "dy": position[1] - previous_position[1],
            }
        previous_position = position
    return None


def main() -> int:
    if len(sys.argv) != 5:
        fail("usage: verify_stoker_exit_pedestrian_route.py EXE ASSETS FIXTURE OUTPUT")
    executable, assets, fixture, output = map(Path, sys.argv[1:])
    output.mkdir(parents=True, exist_ok=True)
    route = fixture / "route.lua"
    gpgx_frames = read_gpgx_frames(fixture / "gpgx_route_frames.tsv")
    route_frame_count = input_log_from_route(route, output / "route.srxkey")
    trace_path = output / "port_route_frames.jsonl"
    actual_path = output / "actual.bmp"

    import os

    env = os.environ.copy()
    env.update({
        "QT_QPA_PLATFORM": "offscreen",
        "SRX_HEADLESS": "1",
        "SDL_AUDIODRIVER": "dummy",
    })
    command = [
        str(executable), "--assets", str(assets), "--headless-replay",
        "--replay-input", str(output / "route.srxkey"),
        "--replay-to-frame", str(route_frame_count),
        "--dump-frame", str(actual_path),
        "--dump-debug-trace", str(trace_path),
    ]
    completed = subprocess.run(command, env=env, check=False, capture_output=True,
                               text=True, timeout=600)
    if completed.returncode != 0:
        fail(f"headless route replay failed ({completed.returncode}): "
             f"{completed.stderr.strip()}")

    source_spawn = parse_source_slots(gpgx_frames[8035])
    source_exit = parse_source_slots(gpgx_frames[10000])
    if set(source_spawn) != {3, 4, 5} or set(source_exit) != {3, 4, 5}:
        fail("independent GPGX route fixture no longer has three active default peds")
    if gpgx_frames[8035]["active_slots"] != "0,3,4,5":
        fail("GPGX source frame 8035 did not publish the three default slots")
    for source_row in gpgx_frames.values():
        if source_row["scene"] != "2":
            continue
        a800_open = (
            int(source_row["FFFBED"], 16) == 5
            and int(source_row["FFDF40"], 16) == 2
            and int(source_row["FFE12C"], 16) == int(source_row["FFF0C8"], 16)
            and int(source_row["FFFC3C"], 16) == 0
        )
        if a800_open:
            fail(f"GPGX A800 gate unexpectedly opened at source frame {source_row['frame']}")

    trace = [json.loads(line) for line in trace_path.read_text(encoding="utf-8").splitlines()]
    scene_rows = [row for row in trace
                  if row.get("app.phase") == "field_ready" and
                  row.get("field.scene") == 2]
    if not scene_rows:
        fail("port did not enter Scene 2 on the cold-boot route")
    for row in scene_rows:
        if row.get("field.mode") != 0 or \
           row.get("field.world.pending_effect") is True or \
           row.get("field.world.active_effect") is True:
            fail(f"port Scene-2 A800 gate facts changed at frame "
                 f"{row.get('app.source_frame')}")
        if any(key.startswith("actor.") and
               key.endswith(".world_activity_script_actor") and value is True
               for key, value in row.items()):
            fail(f"port selected a conditional A800 actor at frame "
                 f"{row.get('app.source_frame')}")
    first_ped_row = next((row for row in scene_rows
                          if set(port_pedestrians(row)) == {3, 4, 5}), None)
    if first_ped_row is None:
        first_count = scene_rows[0].get("field.actor_count")
        fail("port never activated GPGX's three default pedestrian slots; "
             f"first Scene-2 frame {scene_rows[0].get('app.source_frame')} "
             f"has field.actor_count={first_count}")

    # ROM 0x0281E's initial FFDF8A refresh enters 0x02900 and
    # 0x04388/0x043B6 before 0x08C0's 31 warmup services. The independent
    # cold-boot GPGX trace has all three default records active at frame 8035;
    # positions remain fixed through frame 8134, and slot 3 first moves at
    # frame 8138. Require the port to publish them during its first Scene-2
    # field row; its application frame counter uses a
    # different startup phase and is not directly compared to GPGX's counter.
    if first_ped_row is not scene_rows[0]:
        fail("port deferred Scene-2 default actors past initial scene entry; "
             f"first scene row={scene_rows[0].get('app.source_frame')} "
             f"first active row={first_ped_row.get('app.source_frame')}")

    port_spawn = port_pedestrians(first_ped_row)
    # ROM 0x44BC initializes +0x10 to 0x05 and 0x449E adds bit 5 for a
    # root-tagged list entry; GPGX frame 8035 records 0x25 for slots 3-5.
    for slot, source in source_spawn.items():
        actual = port_spawn[slot]
        if actual["x"] != source["x"] or actual["y"] != source["y"]:
            fail(f"slot {slot} initial position {actual['x']:04X},{actual['y']:04X} "
                 f"does not match GPGX {source['x']:04X},{source['y']:04X}")
        if actual["spawn_flags"] != source["spawn_flags"]:
            fail(f"slot {slot} +0x9E flags {actual['spawn_flags']:02X} "
                 f"do not match GPGX {source['spawn_flags']:02X}")
        if (source["entity_flags"] & 0x20) == 0:
            fail(f"GPGX slot {slot} lost the root-tagged +0x10 bit-5 fact")
        if actual["entity_flags"] != source["entity_flags"]:
            fail(f"slot {slot} +0x10 flags {actual['entity_flags']:02X} "
                 f"do not match GPGX {source['entity_flags']:02X}")

    # ROM 0xB610 selects the BE62 (10-unit) vector for a live +0x9E bit-5
    # pedestrian when FFDF84 is clear. The independent GPGX route captures
    # this row's first step as (-10,-10); check the port's first step against
    # that same source observation. Spawn-frame offsets differ, so this checks
    # the vector, not movement cadence or total route timing.
    source_rows = sorted(gpgx_frames.values(), key=lambda row: int(row["frame"]))
    source_first_step = first_movement_step(
        source_rows, 3, parse_source_slots, "frame")
    port_rows = sorted(scene_rows, key=lambda row: int(row["app.source_frame"]))
    port_first_step = first_movement_step(
        port_rows, 3, port_pedestrians, "app.source_frame")
    if source_first_step is None or port_first_step is None:
        fail("GPGX and port route traces must both contain slot 3 movement")
    if (source_first_step["dx"], source_first_step["dy"]) != (-10, -10):
        fail("GPGX Scene-2 slot 3 first movement vector changed from (-10,-10) "
             f"at source frame {source_first_step['frame']}")
    if (port_first_step["dx"], port_first_step["dy"]) != (
            source_first_step["dx"], source_first_step["dy"]):
        fail("port Scene-2 slot 3 first movement vector "
             f"({port_first_step['dx']},{port_first_step['dy']}) does not "
             f"match GPGX ({source_first_step['dx']},{source_first_step['dy']})")

    port_exit = next((row for row in scene_rows
                      if row.get("app.source_frame") == 10001), None)
    if port_exit is None:
        fail("port route trace has no completed frame 10001 after Stoker's exit")
    port_exit_peds = port_pedestrians(port_exit)
    if set(port_exit_peds) != set(source_exit):
        fail("port lost a default pedestrian slot after the Stoker exit route")
    # The same source flags persist in GPGX frame 10000 after AI services;
    # ROM 0x142CE consumes bit 5 to select the actor route-list branch.
    for slot, source in source_exit.items():
        actual = port_exit_peds[slot]
        if actual["entity_flags"] != source["entity_flags"]:
            fail(f"slot {slot} post-exit +0x10 flags "
                 f"{actual['entity_flags']:02X} do not match GPGX "
                 f"{source['entity_flags']:02X}")
    camera_x = int(port_exit["field.camera.x"])
    camera_y = int(port_exit["field.camera.y"])
    visible = [
        slot for slot, actor in port_exit_peds.items()
        if 0 <= (actor["x"] - camera_x) // 16 < 272
        and 0 <= (actor["y"] - camera_y) // 16 < 224
    ]
    source_slot3 = source_exit[3]
    if not (0 <= (source_slot3["x"] - int(gpgx_frames[10000]["camera_x"], 16)) // 16 < 272
            and 0 <= (source_slot3["y"] - int(gpgx_frames[10000]["camera_y"], 16)) // 16 < 224):
        fail("GPGX source slot 3 is not projected within the frame-10000 viewport")
    if not visible:
        fail("port has no pedestrian projected into the viewport after Stoker's exit")
    if not actual_path.is_file():
        fail("headless route replay did not save its actual.bmp frame")
    print("stoker_exit_pedestrian_route=passed source_slots=3,4,5 "
          f"port_visible_slots={','.join(map(str, visible))} "
          f"first_vector={source_first_step['dx']},{source_first_step['dy']} "
          f"gpgx_first_move={source_first_step['frame']} "
          f"port_first_move={port_first_step['frame']} "
          "gpgx_spawn_frame=8035 gpgx_exit_frame=10000")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
