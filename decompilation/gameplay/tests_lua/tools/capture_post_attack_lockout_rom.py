#!/usr/bin/env python3
"""Capture US-ROM action gate and post-action movement timing in GPGX.

This uses the project's headless libretro wrapper at /workspace/shadowrun/emu/retro.py
and a saved field state, then forces one active player record's action fields.
It never writes to the ROM or state fixture.
"""
from __future__ import annotations

import argparse
import csv
import ctypes as C
import hashlib
import importlib.util
import os
from pathlib import Path
import sys


ROM_SHA256 = "92a614491fc3ea12a6df32459cce08b48574b7b814e244c07eb860dc5c6edb7e"
PLAYER = 0xFF0100
SKILL_OFFSETS = {"sorcery": 0x80, "pistols": 0x82,
                 "smgs": 0x83, "shotguns": 0x84}
FRAME_LIMIT = 180


CASES = [
    # The action delay reads +0x78/+0x7B and the first set +0x90 bit 12..14.
    # Weapon codes 1..10 use half mode; codes 11..12 use normal mode.
    {"name": "pistol_skill0", "kind": "weapon", "code": 1,
     "skill": "pistols", "skill_value": 0, "quickness": 3,
     "intelligence": 2, "cyberware": 0},
    {"name": "pistol_skill8", "kind": "weapon", "code": 1,
     "skill": "pistols", "skill_value": 8, "quickness": 3,
     "intelligence": 2, "cyberware": 0},
    {"name": "smg_skill0", "kind": "weapon", "code": 8,
     "skill": "smgs", "skill_value": 0, "quickness": 3,
     "intelligence": 2, "cyberware": 0},
    {"name": "smg_skill8", "kind": "weapon", "code": 8,
     "skill": "smgs", "skill_value": 8, "quickness": 3,
     "intelligence": 2, "cyberware": 0},
    {"name": "shotgun_skill0", "kind": "weapon", "code": 11,
     "skill": "shotguns", "skill_value": 0, "quickness": 3,
     "intelligence": 2, "cyberware": 0},
    {"name": "shotgun_skill8", "kind": "weapon", "code": 11,
     "skill": "shotguns", "skill_value": 8, "quickness": 3,
     "intelligence": 2, "cyberware": 0},
    {"name": "pistol_wr4", "kind": "weapon", "code": 1,
     "skill": "pistols", "skill_value": 4, "quickness": 3,
     "intelligence": 2, "cyberware": 0x1000},
    {"name": "pistol_wr8", "kind": "weapon", "code": 1,
     "skill": "pistols", "skill_value": 4, "quickness": 3,
     "intelligence": 2, "cyberware": 0x2000},
    {"name": "pistol_wr12", "kind": "weapon", "code": 1,
     "skill": "pistols", "skill_value": 4, "quickness": 3,
     "intelligence": 2, "cyberware": 0x4000},
    {"name": "pistol_wr_priority", "kind": "weapon", "code": 1,
     "skill": "pistols", "skill_value": 4, "quickness": 3,
     "intelligence": 2, "cyberware": 0x3000},
    {"name": "pistol_higher_attributes", "kind": "weapon", "code": 1,
     "skill": "pistols", "skill_value": 4, "quickness": 5,
     "intelligence": 4, "cyberware": 0},
    # Negative +0x56 selects 0xAE6E. This forces a cast attempt, then observes
    # the same 0xAF7C/0xAFAE action setup used by the field spell route.
    {"name": "spell_sorcery0", "kind": "spell", "code": 9,
     "skill": "sorcery", "skill_value": 0, "quickness": 3,
     "intelligence": 2, "cyberware": 0},
    {"name": "spell_sorcery8", "kind": "spell", "code": 9,
     "skill": "sorcery", "skill_value": 8, "quickness": 3,
     "intelligence": 2, "cyberware": 0},
    {"name": "spell_wr4", "kind": "spell", "code": 9,
     "skill": "sorcery", "skill_value": 4, "quickness": 3,
     "intelligence": 2, "cyberware": 0x1000},
    {"name": "spell_wr8", "kind": "spell", "code": 9,
     "skill": "sorcery", "skill_value": 4, "quickness": 3,
     "intelligence": 2, "cyberware": 0x2000},
    {"name": "spell_wr12", "kind": "spell", "code": 9,
     "skill": "sorcery", "skill_value": 4, "quickness": 3,
     "intelligence": 2, "cyberware": 0x4000},
]


def u16(value: int) -> int:
    return value & 0xFFFF


def signed_word(value: int) -> int:
    value &= 0xFFFF
    return value - 0x10000 if value & 0x8000 else value


def b(ram, address: int) -> int:
    return int(ram[address & 0xFFFF])


def w(ram, address: int) -> int:
    return (b(ram, address) << 8) | b(ram, address + 1)


def longword(ram, address: int) -> int:
    return (w(ram, address) << 16) | w(ram, address + 2)


def write_byte(raw, address: int, value: int) -> None:
    # retro.py exposes the 68000 RAM through host-endian words.
    raw[(address & 0xFFFF) ^ 1] = value & 0xFF


def expected_action_gate(case: dict) -> int:
    q, intelligence = case["quickness"], case["intelligence"]
    attribute_average = (q + intelligence) // 2  # 0xD81C
    modifier = 0
    for flag, value in ((0x1000, 4), (0x2000, 8), (0x4000, 12)):
        if case["cyberware"] & flag:
            modifier = value
            break  # 0xD7EA checks bits 12, 13, 14 in that order.
    base = 0x6A - max(attribute_average + modifier - 1, 0) * 4
    if case["kind"] == "weapon" and case["code"] <= 10:
        base = base // 2  # 0xB39A -> 0xABFC, signed ASR for positive values.
    return max(base, 0x0F)


def capture_case(emu, state_blob: bytes, case: dict) -> dict:
    emu.pressed.clear()
    emu.load_state(state_blob)
    raw = C.cast(emu.ram_ptr, C.POINTER(C.c_uint8))

    write_byte(raw, PLAYER + 0x78, case["quickness"])
    write_byte(raw, PLAYER + 0x7B, case["intelligence"])
    write_byte(raw, PLAYER + SKILL_OFFSETS[case["skill"]],
               case["skill_value"])
    emu.poke_w(PLAYER + 0x90, case["cyberware"])
    emu.poke_w(PLAYER + 0xDC, 0xFFFF)
    write_byte(raw, PLAYER + 0x56,
               0xFF if case["kind"] == "spell" else 0)
    write_byte(raw, PLAYER + 0x57, case["code"])
    write_byte(raw, PLAYER + 0x4E, 30)
    write_byte(raw, PLAYER + 0x4F, 5)

    before = emu.ram()
    start_xy = (w(before, 0x100), w(before, 0x104))
    before_selector = w(before, PLAYER + 0x16)
    emu.pressed = {"a"}
    emu.run()
    after_action = emu.ram()
    action_selector = w(after_action, PLAYER + 0x16)
    action_gate = signed_word(w(after_action, PLAYER + 0xDC))
    if action_gate < 0 or action_selector == before_selector:
        raise RuntimeError(
            f"{case['name']}: forced A action did not enter its source action path "
            f"(selector {before_selector:04X}->{action_selector:04X}, gate {action_gate})")

    # Release A and hold right. The first changed player coordinate is the
    # source movement re-entry point, even when +0xDC remains nonnegative.
    action_xy = (w(after_action, 0x100), w(after_action, 0x104))
    emu.pressed = {"right"}
    first_move_pass = None
    gate_ready_pass = None
    pose_complete_pass = None
    first_move_gate = None
    for pass_number in range(1, FRAME_LIMIT + 1):
        emu.run()
        ram = emu.ram()
        gate = signed_word(w(ram, PLAYER + 0xDC))
        selector = w(ram, PLAYER + 0x16)
        current_xy = (w(ram, 0x100), w(ram, 0x104))
        if pose_complete_pass is None and selector & 0x8000:
            pose_complete_pass = pass_number
        if gate_ready_pass is None and gate < 0:
            gate_ready_pass = pass_number
        if first_move_pass is None and current_xy != action_xy:
            first_move_pass = pass_number
            first_move_gate = gate
    if first_move_pass is None or gate_ready_pass is None:
        raise RuntimeError(
            f"{case['name']}: movement or action gate did not complete within "
            f"{FRAME_LIMIT} source frames")
    expected_gate = expected_action_gate(case)
    if action_gate != expected_gate:
        raise RuntimeError(
            f"{case['name']}: source +0xDC {action_gate}, formula expected {expected_gate}")
    if gate_ready_pass != action_gate + 1:
        raise RuntimeError(
            f"{case['name']}: observed +0xDC ready pass {gate_ready_pass}, "
            f"expected {action_gate + 1}")

    return {
        "case": case["name"],
        "action_kind": case["kind"],
        "action_code": case["code"],
        "skill": case["skill"],
        "skill_value": case["skill_value"],
        "quickness": case["quickness"],
        "intelligence": case["intelligence"],
        "cyberware_flags": f"0x{case['cyberware']:04X}",
        "source_action_gate": action_gate,
        "next_action_pass": action_gate + 1,
        "observed_gate_ready_pass": gate_ready_pass,
        "action_selector": f"0x{action_selector:04X}",
        "pose_complete_pass": pose_complete_pass or "",
        "first_movement_pass": first_move_pass,
        "gate_at_first_movement": first_move_gate,
        "movement_before_gate_ready": int(first_move_pass < gate_ready_pass),
        "initial_x": start_xy[0],
        "initial_y": start_xy[1],
        "post_action_x": action_xy[0],
        "post_action_y": action_xy[1],
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--rom", type=Path,
                        default=Path("decompilation/input/shadowrun_genesis_us.bin"))
    parser.add_argument("--state", type=Path,
                        default=Path("/workspace/shadowrun/emu/doors/jh0.state"))
    parser.add_argument("--retro", type=Path,
                        default=Path("/workspace/shadowrun/emu/retro.py"))
    parser.add_argument("--core", type=Path,
                        default=Path(os.environ.get(
                            "GPGX_CORE",
                            "/workspace/shadowrun/emu/core/src/genesis_plus_gx_libretro.so")))
    parser.add_argument("--output", type=Path,
                        default=Path("decompilation/gameplay/tests_lua/fixtures/" 
                                     "post_attack_lockout_gpgx.tsv"))
    args = parser.parse_args()

    rom = args.rom.resolve()
    state_path = args.state.resolve()
    retro_path = args.retro.resolve()
    core = args.core.resolve()
    for required in (rom, state_path, retro_path, core):
        if not required.is_file():
            parser.error(f"required capture input is missing: {required}")
    rom_sha = hashlib.sha256(rom.read_bytes()).hexdigest()
    if rom_sha != ROM_SHA256:
        parser.error(f"US ROM SHA-256 mismatch: {rom_sha}")
    state_blob = state_path.read_bytes()
    state_sha = hashlib.sha256(state_blob).hexdigest()

    os.environ["GPGX_CORE"] = str(core)
    spec = importlib.util.spec_from_file_location("srx_retro", retro_path)
    if spec is None or spec.loader is None:
        parser.error(f"could not load retro frontend: {retro_path}")
    retro = importlib.util.module_from_spec(spec)
    sys.modules[spec.name] = retro
    spec.loader.exec_module(retro)

    emu = retro.Emu(str(rom))
    emu.run()
    rows = [capture_case(emu, state_blob, case) for case in CASES]
    args.output.parent.mkdir(parents=True, exist_ok=True)
    fields = list(rows[0])
    with args.output.open("w", newline="") as stream:
        stream.write("# US Genesis GPGX forced-RAM action timing capture; emulator only.\n")
        stream.write(f"# ROM SHA-256: {rom_sha}\n")
        stream.write(f"# Savestate SHA-256: {state_sha}\n")
        stream.write("# Player is record FF0100; Q/I at +0x78/+0x7B, skill at +0x80..+0x8B, cyberware at +0x90.\n")
        stream.write("# Sources: 0xA8EA/+0xDC, 0xABFC, 0xB39A, 0xD7EA/0xD81C, 0xAF7C/0xAFAE, 0x5FDC/0x600E.\n")
        stream.write("# Movement pass counts complete GPGX frames after the action-entry frame; right is held after A is released.\n")
        stream.write("# A positive gate_at_first_movement proves movement resumes before the separate next-action gate.\n")
        writer = csv.DictWriter(stream, fieldnames=fields, delimiter="\t",
                                lineterminator="\n")
        writer.writeheader()
        writer.writerows(rows)
    for row in rows:
        print("{case}: +0xDC={source_action_gate}, next action={next_action_pass}, "
              "first movement={first_movement_pass} (gate {gate_at_first_movement}), "
              "pose complete={pose_complete_pass}".format(**row))
    print(f"wrote {args.output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
