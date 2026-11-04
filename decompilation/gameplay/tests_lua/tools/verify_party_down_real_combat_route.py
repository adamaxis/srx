#!/usr/bin/env python3
"""Check the live NPC-to-player down path against independent GPGX frames."""

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
    raise SystemExit(f"#44 real-combat party-down QA: {message}")


def make_route_input(route_path: Path, output_path: Path, frame_count: int) -> None:
    source = route_path.read_text(encoding="utf-8")
    pattern = re.compile(
        r"\{\s*frame\s*=\s*(\d+)\s*,\s*pad\s*=\s*E\.Pad\.([A-Z_]+)\s*,"
        r"\s*pressed\s*=\s*(true|false)\s*\}"
    )
    events = [
        (int(frame), QT_KEYS[pad], pressed == "true")
        for frame, pad, pressed in pattern.findall(source)
        if pad in QT_KEYS
    ]
    # Continue the real cold-boot route into the nearby encounter, cycle its
    # target, then hold the ordinary attack button. No state or RAM shortcuts.
    events.extend(((12000, QT_KEYS["B"], True),
                   (12001, QT_KEYS["B"], False),
                   (12030, QT_KEYS["A"], True)))
    events.sort()
    if not events or max(event[0] for event in events) >= frame_count:
        fail("route input actions do not fit the requested replay length")
    output_path.write_bytes(
        b"SRXKEY1\0"
        + struct.pack("<QQ", frame_count, len(events))
        + b"".join(
            struct.pack("<QII", frame, key, 0) + bytes((pressed,))
            for frame, key, pressed in events
        )
    )


def source_gpgx_check(path: Path) -> dict[int, dict[str, str]]:
    lines = [line for line in path.read_text(encoding="utf-8").splitlines()
             if line and not line.startswith("#")]
    headers = lines[0].split("\t")
    rows = {
        int(values[0]): dict(zip(headers, values))
        for values in (line.split("\t") for line in lines[1:])
    }
    zero = rows[21092]
    transition = rows[21104]
    timer_done = rows[21163]
    recovered = rows[21181]
    if not (zero["leader_flags"] == "0F" and zero["physical"] == "9" and
            zero["mental"] == "0" and transition["leader_flags"] == "1F" and
            transition["FFE166"] == "003B" and timer_done["FFE166"] == "0000" and
            recovered["leader_flags"] == "0F" and recovered["physical"] == "9" and
            recovered["mental"] == "8"):
        fail("independent GPGX zero/down/countdown/recovery fixture changed")
    return rows


def main() -> int:
    if len(sys.argv) != 5:
        fail("usage: verify_party_down_real_combat_route.py EXE ASSETS FIXTURE OUTPUT")
    executable, assets, fixture, output = map(Path, sys.argv[1:])
    output.mkdir(parents=True, exist_ok=True)
    source_gpgx_check(fixture / "gpgx_party_down_frames.tsv")
    route_path = fixture.parent / "gpgx_stoker_exit_pedestrians" / "route.lua"
    replay_path = output / "route_and_combat.srxkey"
    trace_path = output / "party_down_frames.jsonl"
    actual_path = output / "actual.bmp"
    frame_count = 13000
    make_route_input(route_path, replay_path, frame_count)

    import os

    env = os.environ.copy()
    env.update({
        "QT_QPA_PLATFORM": "offscreen",
        "SRX_HEADLESS": "1",
        "SDL_AUDIODRIVER": "dummy",
    })
    completed = subprocess.run(
        [str(executable), "--assets", str(assets), "--headless-replay",
         "--replay-input", str(replay_path), "--replay-to-frame",
         str(frame_count), "--dump-frame", str(actual_path),
         "--dump-debug-trace", str(trace_path)],
        env=env, check=False, capture_output=True, text=True, timeout=600)
    if completed.returncode != 0:
        fail(f"headless real-route replay failed ({completed.returncode}): "
             f"{completed.stderr.strip()}")

    rows = [json.loads(line) for line in trace_path.read_text(encoding="utf-8").splitlines()]
    rows = [row for row in rows if row.get("app.phase") == "field_ready"]
    leader = lambda row, key: row.get(f"party.runner.0.{key}")
    zeros = [row for row in rows if leader(row, "active") is True and
             (leader(row, "physical_meter") == 0 or
              leader(row, "mental_meter") == 0)]
    if not zeros:
        fail("real encounter did not reduce either leader meter to zero")
    first_zero = zeros[0]
    zero_frame = int(first_zero["app.source_frame"])
    down_rows = [row for row in rows if zero_frame <= int(row["app.source_frame"]) <= zero_frame + 16
                 and (int(leader(row, "source_flags") or 0) & 0x10) != 0]
    if not down_rows:
        fail(f"leader meter reached zero at frame {zero_frame} but ROM +0x10.4 down state did not start")
    down = down_rows[0]
    down_frame = int(down["app.source_frame"])
    countdown_rows = [row for row in rows if down_frame <= int(row["app.source_frame"]) <= down_frame + 2
                      and int(row.get("field.party_wipe_countdown", 0)) > 0]
    if not countdown_rows:
        fail(f"lone leader entered down state at frame {down_frame} without the ROM FFE166 recovery countdown")
    recovery_start = int(countdown_rows[0]["app.source_frame"])
    recovered = next((row for row in rows
                      if int(row["app.source_frame"]) >= recovery_start + 55 and
                      leader(row, "active") is True and
                      (int(leader(row, "source_flags") or 0) & 0x10) == 0 and
                      int(leader(row, "physical_meter") or 0) >= 6 and
                      int(leader(row, "mental_meter") or 0) >= 8), None)
    if recovered is None:
        fail("party wipe did not restore the active leader to ROM meter floors 6/8")
    if not actual_path.is_file():
        fail("headless combat route did not save actual.bmp")

    print("party_down_real_combat_route=passed "
          f"port_zero_frame={zero_frame} down_frame={down_frame} "
          f"countdown_frame={recovery_start} recovery_frame={recovered['app.source_frame']} "
          "gpgx_zero/down/recovery=21092/21104/21181")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
