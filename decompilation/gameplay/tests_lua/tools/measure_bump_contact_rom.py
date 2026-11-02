#!/usr/bin/env python3
"""Measure the US ROM's actor-contact dispatcher with headless Genesis Plus GX.

This controlled RAM probe uses the retained Scene-2 state, marks the live
actor at FF0400 with +0x9E bit 4, and selects Scene 10's valid dialogue row.
It measures the source callback and the separate global activity countdown;
it does not claim that the Scene-2 actor is a naturally talkable NPC.
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


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--rom", default="decompilation/input/shadowrun_genesis_us.bin")
    parser.add_argument("--state", default="/workspace/shadowrun/emu/doors/jh3.state")
    parser.add_argument("--emu-dir", default="/workspace/shadowrun/emu")
    parser.add_argument(
        "--out", default="/workspace/shadowrun/out/bump_contact_rom_measurement.tsv")
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
        "case\tframe\tcontact\tffe176\ttarget_pointer\thandler\tcallback\tcountdown",
    ]

    def capture(case: str, frame: int, contacting: bool) -> None:
        emu.run()
        rows.append(
            f"{case}\t{frame}\t{int(contacting)}\t{byte(0xFFE176)}\t"
            f"0x{longword(0xFFE128):08X}\t"
            f"0x{longword(0xFFDD2C):08X}\t"
            f"0x{longword(0xFFE104):08X}\t"
            f"{word(0xFFE11C)}"
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
        + "# ROM trace: 0xEB92/0xECCC -> 0x21174; global lock 0x1E2E0/0x1E2EC -> 60, decremented by 0x1DD94.\n"
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


if __name__ == "__main__":
    main()
