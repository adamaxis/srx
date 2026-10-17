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
# NTSC Genesis: the Z80 runs at MCLK/15 and one frame is 262 lines of 3420
# master clocks, so a frame is 3420 * 262 / 15 = 59,736 Z80 cycles [C].
# Source frames are mapped to 1/60 s (735 samples) so the rendered assets
# line up with the frame-locked runtime mixer [H].
Z80_TICKS_PER_FRAME = 59_736
# The VDP holds the Z80 /INT line for about one scanline (3420 / 15 = 228
# Z80 cycles) [H].  A pulse that lands in a DI window or an EI shadow is
# taken when the window closes inside that hold; before this was modelled
# the extractor silently dropped VBlanks (about 2% of them for song 0x06),
# which is what made the rendered tempo depend on the cycle budget.
Z80_INT_HOLD_TICKS = 228
# The driver polls the YM2612 status busy bit (bit 7 tests at 0x0891,
# 0x14C8, 0x1503, 0x1516, 0x15B8, 0x15D4 ...) and the offline YM is never
# busy.  Its tempo normally comes from the VBlank flag that the IM 1 handler
# at 0x0038 leaves at 0x003F [C].  Some songs (0x0A, 0x11) instead patch the
# `jr` at 0x02B7 to NOPs (0x02D0 copies 0x02B3/0x02B4 over it), which enables
# the DI busy-wait at 0x02B9..0x02BD: write register 0x27, then spin on status
# bit 0 (Timer A overflow) [C].  Without Timer A the driver spins forever with
# interrupts disabled, so Timer A/B are modelled below.
YM_STATUS_IDLE = 0x00
YM_STATUS_TIMER_A_OVERFLOW = 0x01
YM_STATUS_TIMER_B_OVERFLOW = 0x02
YM_REG_TIMER_A_HIGH = 0x24
YM_REG_TIMER_A_LOW = 0x25
YM_REG_TIMER_B = 0x26
YM_REG_TIMER_CONTROL = 0x27
YM_TIMER_LOAD_A = 0x01
YM_TIMER_LOAD_B = 0x02
YM_TIMER_ENABLE_A = 0x04
YM_TIMER_ENABLE_B = 0x08
YM_TIMER_RESET_A = 0x10
YM_TIMER_RESET_B = 0x20
# YM2612 runs at MCLK/7 and one Timer A count is 144 YM clocks; the Z80 runs
# at MCLK/15, so a count is 144 * 7 / 15 = 67.2 Z80 cycles.  Timer B counts
# 16 times slower [C, YM2612 datasheet timing].  Kept as exact fractions.
YM_TIMER_A_COUNT_NUM, YM_TIMER_A_COUNT_DEN = 144 * 7, 15
YM_TIMER_B_COUNT_NUM, YM_TIMER_B_COUNT_DEN = 16 * 144 * 7, 15
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
                 command_sequence: tuple[int, ...] = (0x10,),
                 repeat_count: int = 1,
                 repeat_interval_frames: int = 0):
        if repeat_count < 1:
            raise ValueError("repeat_count must be at least one")
        if repeat_count > 1 and repeat_interval_frames < 1:
            raise ValueError(
                "repeated command sequences need a positive frame interval")
        machine_type = _load_z80()
        self.machine = machine_type()
        self.rom = rom
        self.bank = 0
        self.ticks = 0
        self.first_command_ticks: int | None = None
        self.writes: list[ChipWrite] = []
        self.loop_start_ticks: int | None = None
        # Keep every qualifying return per source stream.  The first return
        # from one channel is only evidence; the asset pass decides whether
        # successive returns form a synchronized whole-track boundary.
        self.stream_restart_ticks: dict[int, list[int]] = {}
        self._stream_start_bytes: dict[int, int] = {}
        self._ym_register = [0, 0]
        self._timer_a_value = 0
        self._timer_b_value = 0
        self._timer_control = 0
        self._timer_status = YM_STATUS_IDLE
        # Next overflow, in Z80 cycles scaled by the count denominator (15)
        # so the 67.2-cycle counts accumulate without drift.
        self._timer_a_next: int | None = None
        self._timer_b_next: int | None = None
        self._chunk_base = 0
        self._chunk = 0
        self.dropped_interrupts = 0
        self._frame_debt = 0
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
        if queue:
            self.first_command_ticks = self.ticks
            self._queue_bytes(bytes(queue))
            for _ in range(1, repeat_count):
                self._run_frames(repeat_interval_frames)
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
        if 0x4000 <= address <= 0x4003:
            self._update_timers()
            return self._timer_status
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
            if YM_REG_TIMER_A_HIGH <= self._ym_register[0] <= YM_REG_TIMER_CONTROL:
                self._write_timer(self._ym_register[0], value)
            self.writes.append(
                ChipWrite(self.ticks, "ym", 0, self._ym_register[0], value)
            )
        elif address == 0x4003:
            self.writes.append(
                ChipWrite(self.ticks, "ym", 1, self._ym_register[1], value)
            )
        elif address == 0x7F11:
            self.writes.append(ChipWrite(self.ticks, "psg", 0, 0, value))

    def _now_scaled(self) -> int:
        # Exact cycle inside the current run() chunk; ticks_to_stop counts down.
        return (self._chunk_base + self._chunk - self.machine.ticks_to_stop) * YM_TIMER_A_COUNT_DEN

    def _timer_a_period(self) -> int:
        return (1024 - self._timer_a_value) * YM_TIMER_A_COUNT_NUM

    def _timer_b_period(self) -> int:
        return (256 - self._timer_b_value) * YM_TIMER_B_COUNT_NUM

    def _update_timers(self) -> None:
        now = self._now_scaled()
        if self._timer_a_next is not None and now >= self._timer_a_next:
            periods = (now - self._timer_a_next) // self._timer_a_period() + 1
            self._timer_a_next += periods * self._timer_a_period()
            if self._timer_control & YM_TIMER_ENABLE_A:
                self._timer_status |= YM_STATUS_TIMER_A_OVERFLOW
        if self._timer_b_next is not None and now >= self._timer_b_next:
            periods = (now - self._timer_b_next) // self._timer_b_period() + 1
            self._timer_b_next += periods * self._timer_b_period()
            if self._timer_control & YM_TIMER_ENABLE_B:
                self._timer_status |= YM_STATUS_TIMER_B_OVERFLOW

    def _write_timer(self, register: int, value: int) -> None:
        self._update_timers()
        if register == YM_REG_TIMER_A_HIGH:
            self._timer_a_value = (value << 2) | (self._timer_a_value & 0x03)
        elif register == YM_REG_TIMER_A_LOW:
            self._timer_a_value = (self._timer_a_value & 0x3FC) | (value & 0x03)
        elif register == YM_REG_TIMER_B:
            self._timer_b_value = value
        else:
            now = self._now_scaled()
            if value & YM_TIMER_LOAD_A:
                if self._timer_a_next is None:
                    self._timer_a_next = now + self._timer_a_period()
            else:
                self._timer_a_next = None
            if value & YM_TIMER_LOAD_B:
                if self._timer_b_next is None:
                    self._timer_b_next = now + self._timer_b_period()
            else:
                self._timer_b_next = None
            if value & YM_TIMER_RESET_A:
                self._timer_status &= ~YM_STATUS_TIMER_A_OVERFLOW
            if value & YM_TIMER_RESET_B:
                self._timer_status &= ~YM_STATUS_TIMER_B_OVERFLOW
            self._timer_control = value

    def _run_ticks(self, count: int) -> None:
        while count > 0:
            chunk = min(1_000, count)
            self._chunk_base, self._chunk = self.ticks, chunk
            self.machine.ticks_to_stop = chunk
            self.machine.run()
            self.ticks += chunk
            count -= chunk

    def _interrupts_blocked(self) -> bool:
        return (not self.machine.iff1) or bool(self.machine.int_disabled)

    def _run_frames(self, count: int) -> None:
        for _ in range(count):
            # Cycles spent holding /INT belong to the next frame, so every
            # frame stays exactly Z80_TICKS_PER_FRAME long.
            self._run_ticks(Z80_TICKS_PER_FRAME - self._frame_debt)
            held = 0
            while self._interrupts_blocked() and held < Z80_INT_HOLD_TICKS:
                self._chunk_base, self._chunk = self.ticks, 4
                self.machine.ticks_to_stop = 4
                self.machine.run()
                self.ticks += 4
                held += 4
            self._frame_debt = held
            if self._interrupts_blocked():
                self.dropped_interrupts += 1
            else:
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


def write_vgm(path: pathlib.Path, writes: list[ChipWrite], total_ticks: int,
              origin_ticks: int = 0) -> None:
    """Write a VGM 1.70 stream with Genesis YM2612 and PSG clocks."""

    commands = bytearray()
    previous_samples = 0
    for event in writes:
        sample = round(max(0, event.ticks - origin_ticks) * VGM_RATE /
                       Z80_TICKS_PER_FRAME / 60)
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
        round(max(0, total_ticks - origin_ticks) * VGM_RATE /
              Z80_TICKS_PER_FRAME / 60),
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
        help="additional trace duration after the final command; use a long window so source loops can be found",
    )
    parser.add_argument(
        "--repeat-count", type=int, default=1,
        help="queue the command sequence this many times after driver setup",
    )
    parser.add_argument(
        "--repeat-interval-frames", type=int, default=0,
        help="source-frame spacing between repeated command sequences",
    )
    parser.add_argument(
        "--timeline-from-first-command", action="store_true",
        help="place the first queued command at VGM time zero, retaining driver setup writes",
    )
    args = parser.parse_args()
    if args.repeat_count < 1:
        parser.error("--repeat-count must be at least one")
    if args.repeat_count > 1 and args.repeat_interval_frames < 1:
        parser.error("--repeat-interval-frames must be positive when repeating")
    rom = args.rom.read_bytes()
    for track_id in args.ids:
        trace = GenesisSoundMachine(
            rom, track_id, args.sequence, args.repeat_count,
            args.repeat_interval_frames)
        writes = trace.run(args.seconds)
        origin_ticks = 0
        if args.timeline_from_first_command and trace.first_command_ticks is not None:
            origin_ticks = trace.first_command_ticks
        write_vgm(
            args.vgm_dir / f"track-{track_id:03d}.vgm",
            writes,
            trace.ticks,
            origin_ticks,
        )
        loop_ticks = (
            trace.loop_start_ticks - origin_ticks
            if trace.loop_start_ticks is not None
            else None
        )
        loop_seconds = (
            loop_ticks / Z80_TICKS_PER_FRAME / 60
            if loop_ticks is not None
            else None
        )
        loop_samples = (
            round(loop_ticks * VGM_RATE / (Z80_TICKS_PER_FRAME * 60))
            if loop_ticks is not None
            else None
        )
        restart_count = sum(len(points) for points in trace.stream_restart_ticks.values())
        ym_key_on_count = sum(
            write.chip == "ym" and write.register == 0x28 and
            (write.value & 0x0F) != 0
            for write in writes
        )
        suffix = (f", first stream restart at {loop_seconds:.6f}s ({loop_samples} samples), "
                  f"{restart_count} source restart(s) recorded"
                  if loop_seconds is not None else ", one-shot/no loop seen")
        dropped = (f", {trace.dropped_interrupts} VBlank(s) dropped"
                   if trace.dropped_interrupts else "")
        print(
            f"track-{track_id:03d}: {len(writes)} chip writes, "
            f"{ym_key_on_count} YM key-on writes{suffix}{dropped}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
