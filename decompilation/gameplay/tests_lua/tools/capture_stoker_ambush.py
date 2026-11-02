#!/usr/bin/env python3
"""Capture Stoker's direct ambush sequence from the US ROM in headless GPGX.

The temporary ROM copy adds two hooks in verified 0xFF padding and redirects
the original bounded sampler and actor-service entry points:

* The 0x0D3A hook replays the source RNG call and changes only the result for
  caller 0x249CE to 80, selecting the real 0x249C4 ambush branch.
* The 0xA8EA hook invokes 0x249C4 once, then counts individual source actor
  service passes while leaving the original 0xA8EA body intact.

The fixture records the three initialized records and their first 20 source
service snapshots. This is a source capture, not a port implementation.
Run headlessly after sourcing /workspace/tcenv.sh; GPGX_CORE and the
Genesis Plus GX core used by /workspace/shadowrun/emu/retro.py must be set.
"""
import argparse
import ctypes as C
import hashlib
import os
import sys
import tempfile
from pathlib import Path


ROM_SHA = "92a614491fc3ea12a6df32459cce08b48574b7b814e244c07eb860dc5c6edb7e"
STATE_SHA = "c0a6dd624c51800b6144a17f1283c3c94b9e1bb17e379dd3df0e6f5912c2a935"
PLAYER_XY = (0x1CB0, 0x0BB8)
CAMERA_XY = (0x2D20, 0x09E0)
CAPTURE_PASSES = 20
LOGGER = bytes.fromhex(
    "4eb900000d44c0c148402f02242f00040c82000249ce660c303c005033fc005000ff1f00241f4e75")
HOOK = bytes.fromhex(
    "4a7900ff1f10670e427900ff1f104eb9000249c44e754a7900ff1f006706527900ff1f204eb9000138744eb90000a8f04e75")

PROFILES = {
    6: ("weenr2_k08_item01", "item", 1),
    7: ("weenr1_k08_none00", "none", 0),
    8: ("weenr1_k08_none00", "none", 0),
}


def build_harness(rom: bytes) -> bytes:
    image = bytearray(rom)
    for address, length in ((0x1FFE36, len(LOGGER)),
                            (0x1FFE80, len(HOOK))):
        if set(image[address:address + length]) != {0xFF}:
            raise RuntimeError(f"hook area 0x{address:X} is not ROM padding")
    if image[0x0D3A:0x0D40] != bytes.fromhex("6108c0c14840"):
        raise RuntimeError("unexpected original bytes at 0x0D3A")
    if image[0xA8EA:0xA8F0] != bytes.fromhex("4eb900013874"):
        raise RuntimeError("unexpected original bytes at 0xA8EA")
    image[0x0D3A:0x0D40] = bytes.fromhex("4ef9001ffe36")
    image[0x1FFE36:0x1FFE36 + len(LOGGER)] = LOGGER
    image[0xA8EA:0xA8F0] = bytes.fromhex("4ef9001ffe80")
    image[0x1FFE80:0x1FFE80 + len(HOOK)] = HOOK
    return bytes(image)


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--rom", default="decompilation/input/shadowrun_genesis_us.bin")
    parser.add_argument(
        "--state", default="/workspace/shadowrun/emu/doors/jh3.state",
        help="Scene-2 source state used for the matching port QA")
    parser.add_argument("--emu-dir", default="/workspace/shadowrun/emu",
                        help="directory containing retro.py")
    parser.add_argument(
        "--out", default=(
            "decompilation/gameplay/tests_lua/fixtures/"
            "stoker_ambush_gpgx.tsv"))
    args = parser.parse_args()

    os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")
    os.environ.setdefault("SRX_HEADLESS", "1")
    os.environ.setdefault("SDL_AUDIODRIVER", "dummy")
    rom_path, state_path = Path(args.rom), Path(args.state)
    rom, saved_state = rom_path.read_bytes(), state_path.read_bytes()
    if hashlib.sha256(rom).hexdigest() != ROM_SHA:
        raise RuntimeError("US ROM SHA-256 does not match the capture source")
    if hashlib.sha256(saved_state).hexdigest() != STATE_SHA:
        raise RuntimeError("Scene-2 state SHA-256 does not match the capture setup")
    if not os.environ.get("GPGX_CORE"):
        raise RuntimeError("GPGX_CORE must point at the headless Genesis Plus GX core")

    sys.path.insert(0, args.emu_dir)
    from retro import Emu

    patched = tempfile.NamedTemporaryFile(suffix=".bin", delete=False)
    patched.write(build_harness(rom))
    patched.close()
    emu = None
    try:
        emu = Emu(patched.name)
        emu.run()
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

        emu.load_state(saved_state)
        if (word(0xFF0100), word(0xFF0104)) != PLAYER_XY:
            raise RuntimeError("saved player position differs from capture metadata")
        if (word(0xFFDDC6), word(0xFFDDC8)) != CAMERA_XY:
            raise RuntimeError("saved camera position differs from capture metadata")
        write_word(0xFF1F00, 0)
        write_word(0xFF1F10, 1)
        write_word(0xFF1F20, 0)

        for _ in range(8):
            emu.pressed = set()
            emu.run()
            if word(0xFF1F00) == 0x50:
                break
        if word(0xFF1F00) != 0x50:
            raise RuntimeError("0x249C4 test hook did not force roll 80")

        def actor_record(slot: int) -> dict[str, int | str]:
            base = 0xFF0100 + slot * 0x100

            def record_byte(offset: int) -> int:
                return byte(base + offset)

            def record_word(offset: int) -> int:
                return (record_byte(offset) << 8) | record_byte(offset + 1)

            def record_long(offset: int) -> int:
                return (record_word(offset) << 16) | record_word(offset + 2)

            profile, action_kind, action_code = PROFILES[slot]
            return {
                "slot": slot,
                "profile": profile,
                "x": record_word(0x00),
                "y": record_word(0x04),
                "kind": record_word(0x0E),
                "active": record_byte(0x10) & 1,
                "actor_flags": record_byte(0x10),
                "interaction_flags": record_byte(0x11),
                "action_kind": action_kind,
                "action_code": action_code,
                "source_action_kind": record_byte(0x56),
                "source_action_code": record_byte(0x57),
                "source_posture": record_byte(0x4D),
                "spawn_flags": record_byte(0x9E),
                "motion_fn": record_long(0x3A),
                "target_ptr": record_long(0xA8),
                "heading": record_word(0x2E),
                "target_heading": record_word(0x34),
                "behavior_delay": record_word(0xB6),
            }

        initial = [actor_record(slot) for slot in PROFILES
                   if actor_record(slot)["active"] == 1 and
                   actor_record(slot)["kind"] == 8]
        if len(initial) != 3:
            raise RuntimeError(f"expected 3 initialized Weenrs, got {len(initial)}")
        if [record["profile"] for record in initial] != [
                "weenr2_k08_item01", "weenr1_k08_none00",
                "weenr1_k08_none00"]:
            raise RuntimeError("source actor template order differs from 0x1D6E2A")

        rows: list[dict[str, int | str]] = [
            {**record, "pass": 0} for record in initial]
        service_count = word(0xFF1F20)
        frame_count = 0
        while service_count < CAPTURE_PASSES:
            frame_count += 1
            if frame_count > CAPTURE_PASSES * 10:
                raise RuntimeError("timed out waiting for source actor service passes")
            emu.pressed = set()
            emu.run()
            next_count = word(0xFF1F20)
            if next_count == service_count:
                continue
            if next_count != service_count + 1:
                raise RuntimeError(
                    f"actor service count jumped {service_count}->{next_count}")
            service_count = next_count
            for slot in PROFILES:
                record = actor_record(slot)
                if record["active"] != 1 or record["kind"] != 8:
                    raise RuntimeError(f"source actor slot {slot} retired during capture")
                record["pass"] = service_count
                rows.append(record)

        expected_positions = [(0x3300, 0x1100),
                              (0x3300, 0x1300),
                              (0x3300, 0x1500)]
        if [(record["x"], record["y"]) for record in initial] != expected_positions:
            raise RuntimeError("source positions differ from the recorded 0x1E40C candidates")
        if any(record["motion_fn"] != 0xB45C or record["target_ptr"] != 0 or
               record["heading"] != 0 or record["target_heading"] != 0
               for record in rows):
            raise RuntimeError("source AI snapshot differs from idle, unlinked motion")

        output = Path(args.out)
        output.parent.mkdir(parents=True, exist_ok=True)
        columns = (
            "pass", "slot", "profile", "x", "y", "kind", "action_kind",
            "action_code", "source_action_kind", "source_action_code",
            "actor_flags", "interaction_flags", "source_posture", "spawn_flags",
            "motion_fn", "target_ptr", "heading", "target_heading",
            "behavior_delay",
        )
        with output.open("w", newline="") as fixture:
            fixture.write(
                f"# Headless GPGX source capture; US ROM sha256={ROM_SHA}\n")
            fixture.write(
                f"# State sha256={STATE_SHA}; player=1CB0,0BB8; camera=2D20,09E0; "
                f"forced roll=80 at 0x249C4; source service=0xA8EA\n")
            fixture.write("\t".join(columns) + "\n")
            for row in rows:
                values = []
                for key in columns:
                    value = row[key]
                    if key in ("motion_fn", "target_ptr"):
                        values.append(f"0x{int(value):08X}")
                    else:
                        values.append(str(value))
                fixture.write("\t".join(values) + "\n")
        digest = hashlib.sha256(output.read_bytes()).hexdigest()
        print(f"stoker_ambush_source_rows={len(rows)} actors=3 passes={CAPTURE_PASSES}")
        print(f"stoker_ambush_fixture_sha256={digest} path={output}")
    finally:
        if emu is not None:
            emu.lib.retro_unload_game()
        os.unlink(patched.name)


if __name__ == "__main__":
    main()
