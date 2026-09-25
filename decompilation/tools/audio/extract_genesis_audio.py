#!/usr/bin/env python3
"""Offline Genesis sound extraction for Shadowrun.

This tool is intentionally outside the native game and Ghidra conversion
targets.  It boots the ROM's uploaded Z80 sound driver in an offline Z80
machine, supplies the Genesis ROM bank window, records YM2612/SN76489 writes,
and emits standard VGM command streams. A normal VGM renderer can then turn
those streams into PCM for the offline FLAC asset pack used by the Windows
port.

The native executable never imports this module, the Z80 driver, or the VGM
files. The only runtime-facing result is the completed lossless audio asset
tree under the assets directory.
"""

from __future__ import annotations

import argparse
import pathlib
import struct
import sys
from dataclasses import dataclass


ROM_DRIVER_BEGIN = 0x26C5E
ROM_DRIVER_END = 0x284E4
Z80_COMMAND_RING = 0x1B40
Z80_PRODUCER_INDEX = 0x36
Z80_CONSUMER_INDEX = 0x37
Z80_TICKS_PER_FRAME = 100_000
VGM_RATE = 44_100
TRACK_COUNT = 0xBC // 2
# The uploaded driver fetches one decoded event byte through 0x041A.  The
# current channel's stream base is held at 0x0454/0x0455; a later fetch from
# that channel's original base is a source loop boundary, not a renderer
# timeout.  Watching the current base matters because different resources use
# different channel streams to carry the first complete repeat.
STREAM_FETCH_PC = 0x041A
STREAM_DATA_BASE = 0x1D80
STREAM_LOOP_MIN_TICKS = 60 * Z80_TICKS_PER_FRAME


@dataclass(frozen=True)
class ChipWrite:
    ticks: int
    chip: str
    port: int
    register: int
    value: int


def _load_z80():
    try:
        from z80 import Z80Machine
    except ImportError as exc:  # pragma: no cover - command-line dependency
        raise SystemExit(
            "The offline extractor requires the MIT z80 Python package. "
            "Install it with: python -m pip install z80"
        ) from exc
    return Z80Machine


class GenesisSoundMachine:
    """Offline host for the ROM's Z80 driver and Genesis bank window."""

    def __init__(self, rom: bytes, command_argument: int,
                 command_sequence: tuple[int, ...] = (0x10,)):
        machine_type = _load_z80()
        self.machine = machine_type()
        self.rom = rom
        self.bank = 0
        self.ticks = 0
        self.writes: list[ChipWrite] = []
        self.loop_start_ticks: int | None = None
        # Keep every qualifying return per source stream.  The first return
        # from one channel is only evidence; the asset pass decides whether
        # successive returns form a synchronized whole-track boundary.
        self.stream_restart_ticks: dict[int, list[int]] = {}
        self._stream_start_bytes: dict[int, int] = {}
        self._ym_register = [0, 0]
        driver = rom[ROM_DRIVER_BEGIN:ROM_DRIVER_END]
        self.machine.set_memory_block(0, driver)

        # 0x531AA -> 0x52EEC sends FF, 0B, then the four 24-bit ROM
        # addresses in little-endian byte order.  The 68000 writes the
        # producer index; the Z80 owns the consumer index.
        bootstrap = bytes(
            [
                0xFF,
                0x0B,
                0xE4,
                0x84,
                0x02,
                0x95,
                0x95,
                0x02,
                0x09,
                0x96,
                0x02,
                0xBF,
                0x11,
                0x03,
            ]
        )
        self.machine.set_memory_block(Z80_COMMAND_RING, bootstrap)
        self.machine.memory[Z80_PRODUCER_INDEX] = len(bootstrap)
        self.machine.memory[Z80_CONSUMER_INDEX] = 0
        self.machine.pc = 0
        self.machine.sp = 0x1B20

        # Read callbacks are used only for the Genesis ROM window.  Marking
        # all addresses keeps the callback path simple and lets the same
        # callback serve ordinary Z80 RAM reads.
        self.machine.mark_addrs(0, 0x10000, machine_type.READ_MARK)
        for address in (0x4000, 0x4001, 0x4002, 0x4003, 0x6000, 0x7F11):
            self.machine.mark_addr(address, machine_type.WRITE_MARK)
        self.machine.set_read_callback(self._read)
        self.machine.set_write_callback(self._write)

        self._run_frames(4)
        queue = bytearray()
        for opcode in command_sequence:
            queue.extend((0xFF, opcode & 0xFF, command_argument & 0xFF))
        self._queue_bytes(bytes(queue))

    def _read(self, address: int) -> int:
        if (self.machine.pc == STREAM_FETCH_PC and
                address >= STREAM_DATA_BASE):
            stream_base = (
                self.machine.memory[0x454]
                | (self.machine.memory[0x455] << 8)
            )
            stream_byte = self.machine.memory[address]
            if address == stream_base and stream_base not in self._stream_start_bytes:
                self._stream_start_bytes[stream_base] = stream_byte
            # Startup copies/initializes the first stream more than once.
            # The driver also jumps to internal subphrases within a stream. A
            # return to the current channel's original base, with its original
            # marker after the first source second, is the meaningful song
            # restart; short one-shot effects must not be mislabeled loops.
            if (self.ticks >= STREAM_LOOP_MIN_TICKS and
                    address == stream_base and
                    stream_byte == self._stream_start_bytes.get(stream_base)):
                restarts = self.stream_restart_ticks.setdefault(stream_base, [])
                if not restarts or restarts[-1] != self.ticks:
                    restarts.append(self.ticks)
                    if self.loop_start_ticks is None:
                        self.loop_start_ticks = self.ticks
        if address >= 0x8000:
            rom_address = self.bank * 0x8000 + address - 0x8000
            return self.rom[rom_address] if rom_address < len(self.rom) else 0xFF
        return self.machine.memory[address]

    def _write(self, address: int, value: int) -> None:
        if address == 0x6000:
            # Genesis Z80 bank writes shift the low bit into the high end of
            # a nine-bit serial bank register.  This maps bank 5 + 0x1609 to
            # ROM 0x29609, which is the driver's table-2 base.
            self.bank = (self.bank >> 1) | ((value & 1) << 8)
            return
        if address == 0x4000:
            self._ym_register[0] = value
        elif address == 0x4002:
            self._ym_register[1] = value
        elif address == 0x4001:
            self.writes.append(
                ChipWrite(self.ticks, "ym", 0, self._ym_register[0], value)
            )
        elif address == 0x4003:
            self.writes.append(
                ChipWrite(self.ticks, "ym", 1, self._ym_register[1], value)
            )
        elif address == 0x7F11:
            self.writes.append(ChipWrite(self.ticks, "psg", 0, 0, value))

    def _run_frames(self, count: int) -> None:
        for _ in range(count):
            remaining = Z80_TICKS_PER_FRAME
            while remaining:
                chunk = min(1_000, remaining)
                self.machine.ticks_to_stop = chunk
                self.machine.run()
                self.ticks += chunk
                remaining -= chunk
            self.machine.on_handle_active_int()

    def _queue_bytes(self, payload: bytes) -> None:
        index = self.machine.memory[Z80_PRODUCER_INDEX]
        for value in payload:
            self.machine.memory[Z80_COMMAND_RING + index] = value
            index = (index + 1) & 0x3F
        self.machine.memory[Z80_PRODUCER_INDEX] = index

    def run(self, seconds: float) -> list[ChipWrite]:
        self._run_frames(max(1, round(seconds * 60)))
        return self.writes


def _write_u32(buffer: bytearray, offset: int, value: int) -> None:
    struct.pack_into("<I", buffer, offset, value)


def write_vgm(path: pathlib.Path, writes: list[ChipWrite], total_ticks: int) -> None:
    """Write a VGM 1.70 stream with Genesis YM2612 and PSG clocks."""

    commands = bytearray()
    previous_samples = 0
    for event in writes:
        sample = round(event.ticks * VGM_RATE / Z80_TICKS_PER_FRAME / 60)
        wait = max(0, sample - previous_samples)
        while wait:
            amount = min(wait, 0xFFFF)
            commands.extend((0x61, amount & 0xFF, amount >> 8))
            wait -= amount
        if event.chip == "ym":
            commands.extend(
                (0x52 + event.port, event.register & 0xFF, event.value & 0xFF)
            )
        else:
            commands.extend((0x50, event.value & 0xFF))
        previous_samples = sample

    total_samples = max(
        previous_samples,
        round(total_ticks * VGM_RATE / Z80_TICKS_PER_FRAME / 60),
    )
    wait = max(0, total_samples - previous_samples)
    while wait:
        amount = min(wait, 0xFFFF)
        commands.extend((0x61, amount & 0xFF, amount >> 8))
        wait -= amount
    commands.append(0x66)

    header = bytearray(0x40)
    header[0:4] = b"Vgm "
    _write_u32(header, 0x08, 0x00000170)
    _write_u32(header, 0x0C, 3_579_545)
    _write_u32(header, 0x2C, 0)
    _write_u32(header, 0x30, 0)
    _write_u32(header, 0x2C, 7_670_454)
    _write_u32(header, 0x34, 0x0C)
    _write_u32(header, 0x18, total_samples)
    _write_u32(header, 0x04, len(header) + len(commands) - 4)
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(header + commands)


def parse_ids(value: str) -> list[int]:
    if value.lower() == "all":
        return list(range(TRACK_COUNT))
    result = []
    for piece in value.split(","):
        number = int(piece, 0)
        if not 0 <= number < TRACK_COUNT:
            raise argparse.ArgumentTypeError(
                f"track id {number} is outside table-2 range 0..{TRACK_COUNT - 1}"
            )
        result.append(number)
    return result


def parse_sequence(value: str) -> tuple[int, ...]:
    try:
        sequence = tuple(int(piece, 0) for piece in value.split(","))
    except ValueError as exc:
        raise argparse.ArgumentTypeError("audio command sequence is not numeric") from exc
    if not sequence or any(not 0 <= opcode <= 0xFF for opcode in sequence):
        raise argparse.ArgumentTypeError("audio command sequence must contain byte opcodes")
    return sequence


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("rom", type=pathlib.Path)
    parser.add_argument("vgm_dir", type=pathlib.Path)
    parser.add_argument("--ids", default="all", type=parse_ids)
    parser.add_argument(
        "--sequence", default="0x10", type=parse_sequence,
        help="comma-separated Z80 command opcodes, each given the resource id",
    )
    parser.add_argument(
        "--seconds", type=float, default=60.0,
        help="maximum trace duration; use a long window so source loops can be found",
    )
    args = parser.parse_args()
    rom = args.rom.read_bytes()
    for track_id in args.ids:
        trace = GenesisSoundMachine(rom, track_id, args.sequence)
        writes = trace.run(args.seconds)
        write_vgm(
            args.vgm_dir / f"track-{track_id:03d}.vgm",
            writes,
            trace.ticks,
        )
        loop_seconds = (trace.loop_start_ticks / Z80_TICKS_PER_FRAME / 60
                        if trace.loop_start_ticks is not None else None)
        loop_samples = (round(trace.loop_start_ticks * VGM_RATE /
                               (Z80_TICKS_PER_FRAME * 60))
                        if trace.loop_start_ticks is not None else None)
        restart_count = sum(len(points) for points in trace.stream_restart_ticks.values())
        suffix = (f", first stream restart at {loop_seconds:.6f}s ({loop_samples} samples), "
                  f"{restart_count} source restart(s) recorded"
                  if loop_seconds is not None else ", one-shot/no loop seen")
        print(f"track-{track_id:03d}: {len(writes)} chip writes{suffix}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
