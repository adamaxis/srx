#!/usr/bin/env python3
"""Offline self-check for the extractor's frame timing (needs the ROM and
the z80 package; not part of ctest).  Usage: check_extract_timing.py rom.bin"""
import pathlib
import sys

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))
import extract_genesis_audio as E  # noqa: E402


def main() -> int:
    rom = pathlib.Path(sys.argv[1]).read_bytes()
    assert E.Z80_TICKS_PER_FRAME == 3420 * 262 // 15 == 59_736
    machine = E.GenesisSoundMachine(rom, 0x06)
    start = machine.ticks
    machine.run(10.0)
    frames = (machine.ticks - start) // E.Z80_TICKS_PER_FRAME
    assert frames == 600, frames
    assert machine.dropped_interrupts == 0, machine.dropped_interrupts
    for address in range(0x4000, 0x4004):
        assert machine._read(address) == E.YM_STATUS_IDLE
    print("extract timing: 59736 cycles/frame, 600 frames, 0 VBlanks dropped")
    return 0


if __name__ == "__main__":
    sys.exit(main())
