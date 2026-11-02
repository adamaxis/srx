#!/usr/bin/env python3
"""Measure the US ROM's actor-contact dispatcher with headless Genesis Plus GX.

This controlled RAM probe uses the retained Scene-2 state, marks the live
actor at FF0400 with +0x9E bit 4, and selects Scene 10's valid dialogue row.
It measures the source callback and the separate global activity countdown.
An optional input-driven source-actor pass walks from a Scene-10 checkpoint to
the ROM-authored actor; its scene setup remains explicit and is not described
as an unmodified natural save-state encounter.
"""

from __future__ import annotations

import argparse
import ctypes as C
import hashlib
import os
from pathlib import Path
import sys


ROM_SHA256 = "92a614491fc3ea12a6df32459cce08b48574b7b814e244c07eb860dc5c6edb7e"
STATE_SHA256 = "c0a6dd624c51800b6144a17f1283c3c94b9e1bb17e379dd3df0e6f5912c2a935"
CONTACT_ADDRESS = 0xFF0400
CONTACT_FLAG_ADDRESS = CONTACT_ADDRESS + 0x9E
CONTACT_FLAG = 0x10
HANDLER_ADDRESS = 0x21174
SCENE = 10
ACTIVITY_LOCK_FRAMES = 0x3C
SCENE_10_DEFAULT_LIST = 0x17034
SCENE_10_DEFAULT_POSITION = 0x1704A
PLAYER_RECORD_START = (0x0100, 0x0500)  # Scene-10 entry checkpoint used by field QA.


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--rom", default="decompilation/input/shadowrun_genesis_us.bin")
    parser.add_argument("--state", default="/workspace/shadowrun/emu/doors/jh3.state")
    parser.add_argument("--emu-dir", default="/workspace/shadowrun/emu")
    parser.add_argument(
        "--out", default="/workspace/shadowrun/out/bump_contact_rom_measurement.tsv")
    parser.add_argument(
        "--input-contact-out",
        default="/workspace/shadowrun/out/bump_contact_rom_input_contact.tsv")
    parser.add_argument(
        "--input-contact", action="store_true",
        help="walk by pad input into Scene 10's ROM-authored default bit-4 actor")
    args = parser.parse_args()

    os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")
    os.environ.setdefault("SRX_HEADLESS", "1")
    os.environ.setdefault("SDL_AUDIODRIVER", "dummy")
    if not os.environ.get("GPGX_CORE"):
        raise SystemExit("GPGX_CORE must point at the headless Genesis Plus GX core")

    rom_path, state_path = Path(args.rom), Path(args.state)
    rom, saved_state = rom_path.read_bytes(), state_path.read_bytes()
    if hashlib.sha256(rom).hexdigest() != ROM_SHA256:
        raise SystemExit("US ROM SHA-256 does not match the expected source")
    state_hash = hashlib.sha256(saved_state).hexdigest()
    if STATE_SHA256 and state_hash != STATE_SHA256:
        raise SystemExit("Scene-2 state SHA-256 does not match the expected capture")

    def rom_word(address: int) -> int:
        return (rom[address] << 8) | rom[address + 1]

    def rom_longword(address: int) -> int:
        return (rom_word(address) << 16) | rom_word(address + 2)

    if rom_word(SCENE_10_DEFAULT_LIST) != 0x8001 or \
       rom_word(SCENE_10_DEFAULT_LIST + 2) != 0x7236 or \
       rom_longword(SCENE_10_DEFAULT_LIST + 4) != SCENE_10_DEFAULT_POSITION or \
       rom_word(SCENE_10_DEFAULT_POSITION) != 0x0700 or \
       rom_word(SCENE_10_DEFAULT_POSITION + 2) != 0x0500 or \
       rom[SCENE_10_DEFAULT_LIST + 0x0E:SCENE_10_DEFAULT_LIST + 0x10] != b"\x9e\x10":
        raise SystemExit("Scene-10 default actor source row no longer matches the measured fixture")

    sys.path.insert(0, args.emu_dir)
    from retro import Emu

    emu = Emu(str(rom_path))
    emu.load_state(saved_state)
    ram = C.cast(emu.ram_ptr, C.POINTER(C.c_uint8))

    def byte(address: int) -> int:
        return int(ram[(address & 0xFFFF) ^ 1])

    def write_byte(address: int, value: int) -> None:
        ram[(address & 0xFFFF) ^ 1] = value & 0xFF

    def word(address: int) -> int:
        return (byte(address) << 8) | byte(address + 1)

    def write_word(address: int, value: int) -> None:
        write_byte(address, value >> 8)
        write_byte(address + 1, value)

    def longword(address: int) -> int:
        return (word(address) << 16) | word(address + 2)

    player_x, player_y = word(0xFF0100), word(0xFF0104)
    write_word(0xFFDDC4, SCENE)
    write_byte(0xFFE176, 0)
    write_word(0xFFE11C, 0)
    for address in (0xFFE104, 0xFFDD2C, 0xFFE128):
        for offset in range(4):
            write_byte(address + offset, 0)
    write_word(0xFF0100, player_x)
    write_word(0xFF0104, player_y)
    write_word(CONTACT_ADDRESS, player_x)
    write_word(CONTACT_ADDRESS + 4, player_y)
    write_byte(CONTACT_ADDRESS + 0x10, 0x25)
    write_byte(CONTACT_FLAG_ADDRESS, CONTACT_FLAG)

    rows = [
        "case\tframe\tcontact\tffe176\ttarget_pointer\thandler\tcallback\tcountdown\tfff0ae\tfffc12_signed",
    ]

    def capture(case: str, frame: int, contacting: bool) -> None:
        emu.run()
        rows.append(
            f"{case}\t{frame}\t{int(contacting)}\t{byte(0xFFE176)}\t"
            f"0x{longword(0xFFE128):08X}\t"
            f"0x{longword(0xFFDD2C):08X}\t"
            f"0x{longword(0xFFE104):08X}\t"
            f"{word(0xFFE11C)}\t{byte(0xFFF0AE)}\t"
            f"{(word(0xFFFC12) - 0x10000 if word(0xFFFC12) >= 0x8000 else word(0xFFFC12))}"
        )

    # ROM-idle case: first overlap pass should install and invoke 0x21174.
    capture("idle", 1, True)

    # Busy case: the global 60-frame activity lock drains independently of
    # contact. Break contact for frames 20-29, then maintain it through frame 61.
    emu = Emu(str(rom_path))
    emu.load_state(saved_state)
    ram = C.cast(emu.ram_ptr, C.POINTER(C.c_uint8))
    write_word(0xFFDDC4, SCENE)
    write_byte(0xFFE176, 0)
    write_word(0xFFE11C, ACTIVITY_LOCK_FRAMES)
    for address in (0xFFE104, 0xFFDD2C, 0xFFE128):
        for offset in range(4):
            write_byte(address + offset, 0)
    write_word(0xFF0100, player_x)
    write_word(0xFF0104, player_y)
    for frame in range(1, 62):
        contacting = not (20 <= frame < 30)
        # Hold the ROM's separate eligibility gate open so this probe isolates
        # the activity countdown rather than unrelated Scene-2 state changes.
        write_byte(0xFFE176, 0)
        if contacting:
            write_word(CONTACT_ADDRESS, player_x)
            write_word(CONTACT_ADDRESS + 4, player_y)
            write_byte(CONTACT_ADDRESS + 0x10, 0x25)
            write_byte(CONTACT_FLAG_ADDRESS, CONTACT_FLAG)
        else:
            write_word(CONTACT_ADDRESS, (player_x + 0x400) & 0xFFFF)
            write_byte(CONTACT_ADDRESS + 0x10, 0x25)
        capture("busy_with_contact_gap", frame, contacting)

    out_path = Path(args.out)
    out_path.parent.mkdir(parents=True, exist_ok=True)
    out_path.write_text(
        "# US ROM SHA-256: " + ROM_SHA256 + "\n"
        + "# State SHA-256: " + state_hash + "\n"
        + "# Controlled fixture: source Scene-2 state, Scene forced to 10, live FF0400 +0x9E bit 4 set, co-located with player.\n"
        + "# ROM trace: 0xEB92/0xECCC -> 0x21174; global lock 0x1E2E0/0x1E2EC -> 60, decremented by 0x1DD94; FFF0AE and signed FFFC12 sampled from RAM.\n"
        + "# The actor flag, scene, and FFE176 eligibility gate are controlled; this measures dispatch timing, not natural NPC authored data.\n"
        + "\n".join(rows)
        + "\n",
        encoding="ascii",
    )
    print(f"ROM SHA-256: {ROM_SHA256}")
    print(f"state SHA-256: {state_hash}")
    print(f"contact measurement: {out_path}")
    for index in (1, 2, 20, 21, 31, 61, 62):
        print(rows[index])

    if args.input_contact:
        input_emu = Emu(str(rom_path))
        input_emu.load_state(saved_state)
        input_ram = C.cast(input_emu.ram_ptr, C.POINTER(C.c_uint8))

        def input_byte(address: int) -> int:
            return int(input_ram[(address & 0xFFFF) ^ 1])

        def input_write_byte(address: int, value: int) -> None:
            input_ram[(address & 0xFFFF) ^ 1] = value & 0xFF

        def input_word(address: int) -> int:
            return (input_byte(address) << 8) | input_byte(address + 1)

        def input_write_word(address: int, value: int) -> None:
            input_write_byte(address, value >> 8)
            input_write_byte(address + 1, value)

        def input_longword(address: int) -> int:
            return (input_word(address) << 16) | input_word(address + 2)

        # Source-seed the documented scene actor and start point, then use
        # controller input for every movement frame. The overlap and handler
        # addresses are never written by this pass.
        input_write_word(0xFFDDC4, SCENE)
        input_write_word(0xFF0100, PLAYER_RECORD_START[0])
        input_write_word(0xFF0104, PLAYER_RECORD_START[1])
        input_write_word(CONTACT_ADDRESS,
                         rom_word(SCENE_10_DEFAULT_POSITION))
        input_write_word(CONTACT_ADDRESS + 4,
                         rom_word(SCENE_10_DEFAULT_POSITION + 2))
        input_write_byte(CONTACT_ADDRESS + 0x10, 0x25)
        input_write_byte(CONTACT_FLAG_ADDRESS, CONTACT_FLAG)
        for address in (0xFF0500, 0xFF0600):
            input_write_byte(address + 0x10, 0)
        input_write_byte(0xFFE176, 0)
        input_write_word(0xFFE11C, 0)
        for address in (0xFFE104, 0xFFDD2C, 0xFFE128):
            for offset in range(4):
                input_write_byte(address + offset, 0)

        input_rows = [
            "frame\tinput\tplayer_x\tplayer_y\tactor_x\tactor_y\t"
            "overlap\tcontact_actor\tcallback\tcountdown\tfff0ae\tfffc12_signed"
        ]
        first_overlap_frame = None
        dispatch_frame = None
        for frame in range(1, 181):
            input_emu.pressed = {"right"}
            input_emu.run()
            player_x = input_word(0xFF0100)
            player_y = input_word(0xFF0104)
            actor_x = input_word(CONTACT_ADDRESS)
            actor_y = input_word(CONTACT_ADDRESS + 4)
            # Match 0xEB92's unlinked half-open +/-0x40 axis window.
            lower_x = (actor_x - 0x40) & 0xFFFF
            upper_x = (actor_x + 0x40) & 0xFFFF
            lower_y = (actor_y - 0x40) & 0xFFFF
            upper_y = (actor_y + 0x40) & 0xFFFF
            in_overlap = (lower_x <= player_x < upper_x and
                          lower_y <= player_y < upper_y)
            if in_overlap and first_overlap_frame is None:
                first_overlap_frame = frame
            handler = input_longword(0xFFE104)
            if handler == HANDLER_ADDRESS and dispatch_frame is None:
                dispatch_frame = frame
            input_rows.append(
                f"{frame}\tright\t0x{player_x:04X}\t0x{player_y:04X}\t"
                f"0x{actor_x:04X}\t0x{actor_y:04X}\t"
                f"{int(in_overlap)}\t0x{input_longword(0xFFE128):08X}\t"
                f"0x{handler:08X}\t{input_word(0xFFE11C)}\t"
                f"{input_byte(0xFFF0AE)}\t"
                f"{(input_word(0xFFFC12) - 0x10000 if input_word(0xFFFC12) >= 0x8000 else input_word(0xFFFC12))}"
            )
            if dispatch_frame is not None:
                break

        input_out = Path(args.input_contact_out)
        input_out.parent.mkdir(parents=True, exist_ok=True)
        input_out.write_text(
            "# US ROM SHA-256: " + ROM_SHA256 + "\n"
            + "# State SHA-256: " + state_hash + "\n"
            + "# Fixture: saved Scene-2 state; Scene 10 and the default actor at 0x17034/0x17236 are source-seeded; runner starts at the Scene-10 entry checkpoint (0x0100,0x0500).\n"
            + "# Movement: right-pad input each frame; overlap coordinates and callback pointers are not written after setup.\n"
            + "# Trace: 0xEB92/0xECCC; gate/lock 0x1E2E0/0x1E2EC; handler 0x21174; Scene-10 default variant 4.\n"
            + f"# first_overlap_frame={first_overlap_frame}; handler_frame={dispatch_frame}; delta={None if first_overlap_frame is None or dispatch_frame is None else dispatch_frame - first_overlap_frame}\n"
            + "\n".join(input_rows) + "\n",
            encoding="ascii",
        )
        print(f"input-driven source actor measurement: {input_out}")
        print(f"first overlap: {first_overlap_frame}; handler: {dispatch_frame}")


if __name__ == "__main__":
    main()
