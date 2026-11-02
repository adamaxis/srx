#!/usr/bin/env python3
"""Reduce the temporary A3 Genesis Plus GX action capture to fight summaries."""

from __future__ import annotations

import argparse
import hashlib
from pathlib import Path
import zlib


ROM_SHA256 = "92a614491fc3ea12a6df32459cce08b48574b7b814e244c07eb860dc5c6edb7e"
CAPTURE_SHA256 = "b8aa5bdd9fc1a9f7cf0d44011c6b58bd56a8d471be4a95f188d9660a8f99edbe"
HEADER = (
    "mode", "seed", "action", "actor_to_target_slot", "item_code",
    "seed_before", "attack_faces", "attack_successes", "defense_faces",
    "defense_successes", "damage", "physical_before", "physical_after",
    "mental_before", "mental_after", "seed_after",
)


def digest(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def build(capture_path: Path, rom_path: Path) -> list[str]:
    capture_hash = digest(capture_path)
    rom_hash = digest(rom_path)
    if capture_hash != CAPTURE_SHA256:
        raise SystemExit(f"unexpected capture SHA-256: {capture_hash}")
    if rom_hash != ROM_SHA256:
        raise SystemExit(f"unexpected US ROM SHA-256: {rom_hash}")

    fights: dict[tuple[str, int], dict[str, object]] = {}
    lines = capture_path.read_text(encoding="ascii").splitlines()
    try:
        header_index = next(i for i, line in enumerate(lines)
                            if line.split("\t") == list(HEADER))
    except StopIteration as exc:
        raise SystemExit("capture header does not match A3 format") from exc

    for line_number, line in enumerate(lines[header_index + 1:],
                                       start=header_index + 2):
        if not line or line.startswith("#"):
            continue
        values = line.split("\t")
        if len(values) != len(HEADER):
            raise SystemExit(f"capture row {line_number} has {len(values)} fields")
        row = dict(zip(HEADER, values))
        key = (row["mode"], int(row["seed"]))
        if key[0] not in ("P", "E"):
            raise SystemExit(f"capture row {line_number} has unknown mode {key[0]}")
        fight = fights.setdefault(key, {
            "rows": [],
            "meters": {"A": [10, 10], "B": [10, 10]},
            "crc": 0,
        })
        rows = fight["rows"]
        assert isinstance(rows, list)
        rows.append(row)
        fight["crc"] = zlib.crc32((line + "\n").encode("ascii"),
                                  int(fight["crc"]))
        first_slot, second_slot = (0, 6) if key[0] == "P" else (6, 7)
        actor_slot, target_slot = map(int, row["actor_to_target_slot"].split(">"))
        meters = fight["meters"]
        assert isinstance(meters, dict)
        side = "A" if target_slot == first_slot else "B"
        if target_slot not in (first_slot, second_slot):
            raise SystemExit(f"capture row {line_number} uses an unexpected slot")
        meters[side] = [int(row["physical_after"]), int(row["mental_after"])]
        if actor_slot == target_slot:
            raise SystemExit(f"capture row {line_number} targets its actor")

    output = [
        "# [C] Source ROM SHA-256: " + ROM_SHA256,
        "# [C] Raw capture: /workspace/shadowrun/out/a3_emulator_combat_capture.tsv",
        "# [C] Raw capture SHA-256: " + CAPTURE_SHA256,
        "# Capture acquisition: headless Genesis Plus GX via /workspace/shadowrun/emu/retro.py;",
        "# temporary Python driver redirected 0x0D3A to a RAM logger and host-stepped calls",
        "# into the original 0x55640/0x55948 combat routines. The one-off driver/argv was not retained.",
        "# Capture-to-fixture command (using the retained raw capture):",
        "# python3 decompilation/gameplay/tests_lua/tools/build_a3_emulator_fixture.py \\",
        "#   --capture /workspace/shadowrun/out/a3_emulator_combat_capture.tsv \\",
        "#   --rom decompilation/input/shadowrun_genesis_us.bin \\",
        "#   --output decompilation/gameplay/tests_lua/fixtures/a3_emulator_fight_summary.tsv",
        "# Per-seed CRC-32 covers each captured 16-field action row, in action order, including",
        "# dice faces, successes, damage, meters, slots, and RNG seeds.",
        "mode\tseed_hex\tactions\twinner\tA_physical\tB_physical\tfinal_seed\taction_crc32",
    ]
    for (mode, seed), fight in sorted(fights.items()):
        rows = fight["rows"]
        meters = fight["meters"]
        assert isinstance(rows, list) and rows
        assert isinstance(meters, dict)
        first_slot, second_slot = (0, 6) if mode == "P" else (6, 7)
        first_physical = int(meters["A"][0])
        second_physical = int(meters["B"][0])
        if first_physical > 0 and second_physical > 0:
            winner = "D"
        elif first_physical > 0:
            winner = "A"
        else:
            winner = "B"
        final_seed = rows[-1]["seed_after"]
        output.append("\t".join((
            mode, f"{seed:04X}", str(len(rows)), winner,
            str(first_physical), str(second_physical), final_seed,
            f"{int(fight['crc']) & 0xFFFFFFFF:08X}",
        )))
    return output


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--capture", required=True, type=Path)
    parser.add_argument("--rom", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    args = parser.parse_args()
    rows = build(args.capture, args.rom)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text("\n".join(rows) + "\n", encoding="ascii")


if __name__ == "__main__":
    main()
