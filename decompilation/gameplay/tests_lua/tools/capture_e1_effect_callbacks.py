#!/usr/bin/env python3
"""Capture E1b effect callback branches in headless Genesis Plus GX.

The source ROM remains unmodified. A temporary capture-only copy wraps 0x0D3A
with the A3 RAM logger and calls the real 0x1597E sleep spread scan once from
the 0xA8EA actor pass. The driver then advances the original actor/render loop
one emulator frame at a time, exercising 0x52BE/0x5322 and its active-record
fan-out checks.

Set GPGX_CORE to the Genesis Plus GX libretro core. No display, audio, or asset
extractor is used.
"""
import argparse
import csv
import ctypes as C
import hashlib
import json
import os
import subprocess
import sys
import tempfile
from pathlib import Path

ROM_SHA = "92a614491fc3ea12a6df32459cce08b48574b7b814e244c07eb860dc5c6edb7e"
STATE_SHA = "b9bc8cfbe6341aa93be083c82706977c374e302e4ef01fee42760842eb245019"
RNG_LOG_PTR = 0xFF1FF4
ROM_DEFAULT = "/workspace/srx_int/decompilation/input/shadowrun_genesis_us.bin"
STATE_DEFAULT = "/workspace/shadowrun/emu/doors/hw2.state"
RNG_LOGGER = bytes.fromhex(
    "4eb900000d44c0c1484048c040e72f012f022f08242f000e207c00ff1ff4"
    "205020c2242f002620c230c130c0343900fff9f830c223c800ff1ff4205f"
    "241f221f46df4e75")
ACTOR_SERVICE_HOOK = bytes.fromhex(
    "4a3900ff1f0067000022423900ff1f0048e7fffe207900ff1f04227900ff1f08"
    "4eb90001597e4cdf7fff4eb9000138744ef90000a8f0")


def build_instrumented_rom(rom: bytes, actor_scan_hook: bool = False,
                          rng_logger: bool = False) -> bytes:
    hooked = bytearray(rom)
    if rng_logger and hooked[0x0D3A:0x0D40] != bytes.fromhex("6108c0c14840"):
        raise SystemExit("unexpected US-ROM bytes at 0x0D3A")
    payloads = [(0x1FFE36, RNG_LOGGER)]
    if not rng_logger:
        payloads = []
    if actor_scan_hook:
        if hooked[0xA8EA:0xA8F0] != bytes.fromhex("4eb900013874"):
            raise SystemExit("unexpected US-ROM bytes at 0xA8EA")
        payloads.append((0x1FFE80, ACTOR_SERVICE_HOOK))
    for address, payload in payloads:
        if hooked[address:address + len(payload)] != b"\xFF" * len(payload):
            raise SystemExit(f"ROM hook area 0x{address:X} is not padding")
    if rng_logger:
        # The RNG helper replays its original 0x0D44/multiply/scale
        # instructions while recording both the immediate caller and the
        # enclosing source return address, plus result and post-seed. The
        # enclosing address is 0x559CE for 0x55B78's d6. Use FF1FF4 so source
        # callback scratch at FF1FF0 cannot overwrite the log cursor.
        hooked[0x0D3A:0x0D40] = bytes.fromhex("4ef9001ffe36")
        logger = RNG_LOGGER.replace(bytes.fromhex("00ff1ff0"),
                                    bytes.fromhex("00ff1ff4"))
        hooked[0x1FFE36:0x1FFE36 + len(logger)] = logger
    if actor_scan_hook:
        # The one-shot A8EA hook calls the original 0x1597E scan with seeded
        # A0/A1, then resumes the actor service at 0xA8F0.
        hooked[0xA8EA:0xA8F0] = bytes.fromhex("4ef9001ffe80")
        hooked[0x1FFE80:0x1FFE80 + len(ACTOR_SERVICE_HOOK)] = ACTOR_SERVICE_HOOK
    return bytes(hooked)


class Capture:
    def __init__(self, rom_path: Path, state: bytes,
                 actor_scan_hook: bool = False,
                 rng_logger: bool = False,
                 state_path: Path | None = None):
        os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")
        os.environ.setdefault("SRX_HEADLESS", "1")
        os.environ.setdefault("SDL_AUDIODRIVER", "dummy")
        sys.path.insert(0, "/workspace/shadowrun/emu")
        from retro import Emu

        self.rom_path = rom_path
        self.state = state
        self.state_path = state_path
        self.patched_rom_path = None
        if actor_scan_hook or rng_logger:
            with tempfile.NamedTemporaryFile(suffix=".bin", delete=False) as patched:
                patched.write(build_instrumented_rom(
                    rom_path.read_bytes(), actor_scan_hook, rng_logger))
                self.patched_rom_path = Path(patched.name)
        rom_for_emu = self.patched_rom_path or rom_path
        self.emu = Emu(str(rom_for_emu))
        self.emu.run()
        self.raw = C.cast(self.emu.ram_ptr, C.POINTER(C.c_uint8))

    def b(self, address: int) -> int:
        return int(self.raw[(address & 0xFFFF) ^ 1])

    def wb(self, address: int, value: int) -> None:
        self.raw[(address & 0xFFFF) ^ 1] = value & 0xFF

    def w(self, address: int) -> int:
        return (self.b(address) << 8) | self.b(address + 1)

    def ww(self, address: int, value: int) -> None:
        self.wb(address, value >> 8)
        self.wb(address + 1, value)

    def l(self, address: int) -> int:
        return (self.w(address) << 16) | self.w(address + 2)

    def wl(self, address: int, value: int) -> None:
        self.ww(address, value >> 16)
        self.ww(address + 2, value)

    @staticmethod
    def record(slot: int) -> int:
        return 0xFF0100 + slot * 0x100

    def reset(self, active_slots, seed: int = 1) -> None:
        self.emu.load_state(self.state)
        self.wl(RNG_LOG_PTR, 0xFF5000)
        self.wb(0xFF1F00, 0)
        active_slots = set(active_slots)
        for slot in range(16):
            actor = self.record(slot)
            if slot not in active_slots:
                self.wb(actor + 0x10, 0)
        for slot in active_slots:
            actor = self.record(slot)
            self.wl(actor + 0x3A, 0x49A0)
            self.ww(actor + 0x3E, slot)
        self.ww(0xFFF9F8, seed)

    def caster(self, code: int) -> int:
        caster = self.record(0)
        self.wb(caster + 0x10, 0)
        self.wb(caster + 0x57, code)
        self.wb(caster + 0x9D, 2)
        self.wb(caster + 0x9F, 4)
        self.wb(caster + 0x77, 2)
        self.wb(caster + 0x7C, 4)
        return caster

    def target(self, slot: int, code: int, kind: int = 8,
               flags: int = 0x25, marker: int | None = None) -> int:
        actor = self.record(slot)
        self.wb(actor + 0x10, flags)
        self.ww(actor + 0x0C, 0x25)
        self.ww(actor + 0x0E, kind)
        self.wb(actor + 0xB2, code)
        self.wb(actor + 0xDE, code if marker is None else marker)
        self.wb(actor + 0x02, 5)
        self.wb(actor + 0x03, 0)
        self.ww(actor + 0x32, 0)
        self.ww(actor + 0x2A, 0x4000)
        self.wl(actor + 0xD8, self.record(0))
        self.wb(actor + 0x4C, 3)
        self.wb(actor + 0x9C, 6)
        self.wb(actor + 0x77, 4)
        self.wb(actor + 0x7C, 1)
        self.wb(actor + 0x9B, 0x0A)
        return actor

    def frame(self) -> None:
        self.emu.pressed = set()
        self.emu.run()

    def rng_calls(self) -> list[dict]:
        end = self.l(RNG_LOG_PTR) & 0xFFFF
        calls = []
        for offset in range(0x5000, end, 14):
            address = 0xFF0000 + offset
            calls.append({
                "immediate_caller": f"{self.l(address):06X}",
                "caller": f"{self.l(address + 4):06X}",
                "bound": self.w(address + 8),
                "result": self.w(address + 10),
                "seed_after": f"{self.w(address + 12):04X}",
            })
        return calls

    def close(self) -> None:
        if self.patched_rom_path is not None:
            self.patched_rom_path.unlink(missing_ok=True)

    def run_until_callback_clears(self, actor: int, maximum: int = 100):
        frames = 0
        seed_terminal = self.w(0xFFF9F8)
        while frames < maximum and self.b(actor + 0xB2) != 0:
            seed_terminal = self.w(0xFFF9F8)
            self.frame()
            frames += 1
        return frames, seed_terminal

    def row(self, case: str, entry: str, heartbeats: int, seed_before: int,
            seed_terminal: int, seed_after: int, actor: int,
            details: dict) -> dict:
        return {
            "case": case,
            "entry": entry,
            "heartbeats": heartbeats,
            "seed_before": f"{seed_before:04X}",
            "seed_terminal": f"{seed_terminal:04X}",
            "seed_after": f"{seed_after:04X}",
            "physical_before": 3,
            "physical_after": self.b(actor + 0x4C),
            "mental_before": 6,
            "mental_after": self.b(actor + 0x9C),
            "caster_physical_after": self.b(self.record(0) + 0x4C),
            "caster_mental_after": self.b(self.record(0) + 0x9C),
            "details": json.dumps(details, sort_keys=True, separators=(",", ":")),
            "selected_slots": ",".join(
                str(slot) for slot in details.get("selected_slots", [])),
            "rng_calls": json.dumps(self.rng_calls(), sort_keys=True,
                                    separators=(",", ":")),
        }

    def code2_resistance(self) -> dict:
        self.reset((0, 6))
        caster = self.caster(2)
        actor = self.target(6, 2, kind=8, marker=0)
        self.wb(actor + 0x10, 0x25)
        self.ww(0xFFE186, 0)
        for _ in range(30):
            self.frame()
        seed_terminal = self.w(0xFFF9F8)
        self.wb(actor + 0x10, 0x2D)  # Set source +0x10 bit 3 for 0x5152.
        self.frame()
        return self.row("code2_bit3_resistance", "0x5152/0x55B78", 31,
                        1, seed_terminal, self.w(0xFFF9F8), actor,
                        {"target_flags10": f"{self.b(actor + 0x10):02X}",
                         "target_behavior": f"{self.l(actor + 0x3A):08X}",
                         "caster": f"{caster:08X}"})

    def code2_type15(self) -> dict:
        self.reset((0, 6))
        caster = self.caster(2)
        actor = self.target(6, 2, kind=0x0F, marker=0)
        # 0xAFF2 returns immediately when target +0x10 bit 4 is set. Together
        # with type 15, this is the actual pose-suppression arm at 0x51C8.
        self.wb(actor + 0x10, 0x35)
        self.ww(0xFFE186, 3)
        seed_before = self.w(0xFFF9F8)
        frames, seed_terminal = self.run_until_callback_clears(actor)
        return self.row("code2_type15_pose_suppression", "0x5152/0x51C8",
                        frames, seed_before, seed_terminal,
                        self.w(0xFFF9F8),
                        actor,
                        {"target_type": f"{self.w(actor + 0x0E):04X}",
                         "target_flags10": f"{self.b(actor + 0x10):02X}",
                         "target_behavior": f"{self.l(actor + 0x3A):08X}",
                         "caster": f"{caster:08X}"})

    def code5(self, case: str, candidates, dead_slot=None,
              kill_after_transfer=False) -> dict:
        slots = {0, 6} | {int(item["slot"]) for item in candidates}
        # Libretro's core is process-global, so run the real 0x1597E scan in
        # an isolated process and leave this ordinary-ROM callback untouched.
        scan_marked = source_scan_marks(self.rom_path, self.state_path,
                                        self.state, candidates)

        self.reset(slots, seed=0x9F2D)
        self.caster(5)
        source = self.target(6, 5, marker=0)
        candidate_details = []
        for item in candidates:
            slot = int(item["slot"])
            actor = self.target(slot, 0, marker=0)
            route_cost = int(item["route_cost"])
            self.wb(actor + 0xDE, 5 if scan_marked[slot] else 0)
            candidate_details.append({
                "slot": slot,
                "route_cost": f"{route_cost:03X}",
                "initially_active": item.get("active", True),
                "marked_by_rom_scan": scan_marked[slot],
            })

        seed_before = self.w(0xFFF9F8)
        seed_terminal = seed_before
        for heartbeat in range(1, 63):
            if dead_slot is not None and heartbeat == 62:
                self.wb(self.record(dead_slot) + 0x10, 0)
            if heartbeat == 62:
                seed_terminal = self.w(0xFFF9F8)
            self.frame()
        seed_after_terminal = self.w(0xFFF9F8)
        selected = [int(item["slot"]) for item in candidates
                    if self.b(self.record(int(item["slot"])) + 0xB2) == 5]
        for detail in candidate_details:
            actor = self.record(detail["slot"])
            flags = self.b(actor + 0x10)
            detail["marked_after_scan"] = self.b(actor + 0xDE) == 5
            detail["active_before_fanout"] = bool(flags & 1) and not bool(flags & 0x40)

        second_transfer = []
        if case == "code5_several_marked":
            for _ in range(62):
                self.frame()
            second_transfer = [int(item["slot"]) for item in candidates
                               if self.b(self.record(int(item["slot"])) +
                                         0xB2) == 5]

        if kill_after_transfer and selected:
            self.wb(self.record(selected[0]) + 0x10, 0)
            for _ in range(20):
                self.frame()
            candidate_details.append({
                "dead_mid_fanout_slot": selected[0],
                "stalled_action": self.b(self.record(selected[0]) + 0xB2),
                "later_slots_started": [
                    int(item["slot"]) for item in candidates[1:]
                    if self.b(self.record(int(item["slot"])) + 0xB2) == 5],
            })

        return self.row(case, "0x52BE/0x5322", 62, seed_before,
                        seed_terminal, seed_after_terminal, source,
                        {"candidates": candidate_details,
                         "selected_slots": selected,
                         "following_transfer_slots": second_transfer,
                         "source_marker_after": self.b(source + 0xDE),
                         "source_phase": self.b(source + 3)})

    def code13(self, case: str, expired: bool, neighbor: bool,
               auxiliary: bool) -> dict:
        active_slots = (0, 6, 7) if neighbor or auxiliary else (0, 6)
        self.reset(active_slots)
        self.caster(13)
        source = self.target(6, 13, marker=0)
        self.ww(source + 0x32, 3)
        self.ww(0xFFDFF6, 0xFFFF)
        self.ww(0xFFDFF8, 0xFFFF if expired else 0)
        self.ww(0xFFE004, 0)
        self.ww(0xFFE002, 0)
        slot3 = 0xFFD86C + 3
        self.wb(slot3, 1)
        clone = self.record(7)
        if neighbor:
            self.target(7, 0, marker=13)
            self.ww(clone + 0x3E, 7)
        elif auxiliary:
            self.target(7, 0, marker=0)
            self.wl(0xFFDFFA, clone)
        seed_before = self.w(0xFFF9F8)
        frames, seed_terminal = self.run_until_callback_clears(source,
                                                                maximum=30)
        return self.row(case, "0x53C4/0x5494", frames, seed_before,
                        seed_terminal, self.w(0xFFF9F8), source,
                        {"expired": expired,
                         "shared_gate": self.w(0xFFDFF6),
                         "shared_delay": self.w(0xFFDFF8),
                         "effect_slot3": self.b(slot3),
                         "clone_slot": 7,
                         "clone_code": self.b(clone + 0xB2),
                         "clone_countdown": self.b(clone + 2),
                         "clone_phase": self.b(clone + 3),
                         "clone_effect_slot": self.w(clone + 0x32),
                         "clone_caster": f"{self.l(clone + 0xD8):08X}"})


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--rom", type=Path, default=Path(ROM_DEFAULT))
    parser.add_argument("--state", type=Path, default=Path(STATE_DEFAULT))
    parser.add_argument("--out", type=Path)
    parser.add_argument("--scan-only", action="store_true")
    parser.add_argument("--scan-spec", type=str)
    parser.add_argument("--scan-out", type=Path)
    args = parser.parse_args()
    rom_bytes = args.rom.read_bytes()
    state_bytes = args.state.read_bytes()
    if hashlib.sha256(rom_bytes).hexdigest() != ROM_SHA:
        raise SystemExit("US ROM SHA-256 mismatch")
    if hashlib.sha256(state_bytes).hexdigest() != STATE_SHA:
        raise SystemExit("Genesis Plus GX state SHA-256 mismatch")

    if args.scan_only:
        if args.scan_spec is None or args.scan_out is None:
            raise SystemExit("--scan-only requires --scan-spec and --scan-out")
        scan_only(args.rom, args.state, state_bytes,
                  json.loads(args.scan_spec), args.scan_out)
        return

    if args.out is None:
        raise SystemExit("normal capture requires --out")

    capture = Capture(args.rom, state_bytes, rng_logger=True,
                      state_path=args.state)
    rows = [
        capture.code2_resistance(),
        capture.code2_type15(),
        capture.code5("code5_zero_marked", [
            {"slot": 7, "route_cost": 0x500}]),
        capture.code5("code5_one_marked", [{"slot": 7, "route_cost": 0x100}]),
        capture.code5("code5_several_marked", [
            {"slot": 7, "route_cost": 0x100},
            {"slot": 8, "route_cost": 0x300}]),
        capture.code5("code5_route_cost_at_500", [
            {"slot": 7, "route_cost": 0x500}]),
        capture.code5("code5_route_cost_above_500", [
            {"slot": 7, "route_cost": 0x501}]),
        capture.code5("code5_dead_before_fanout", [
            {"slot": 7, "route_cost": 0x100},
            {"slot": 8, "route_cost": 0x200}], dead_slot=7),
        capture.code5("code5_dead_mid_fanout", [
            {"slot": 7, "route_cost": 0x100},
            {"slot": 8, "route_cost": 0x200}],
            kill_after_transfer=True),
        capture.code13("code13_expired_cleanup", True, False, False),
        capture.code13("code13_child_clone", False, True, False),
        capture.code13("code13_auxiliary_clone", False, False, True),
    ]
    args.out.parent.mkdir(parents=True, exist_ok=True)
    with args.out.open("w", newline="") as output:
        output.write("# US Genesis Plus GX capture; source ROM unmodified; temporary RAM-logger/0x1597E call hooks only.\n")
        output.write(f"# ROM_SHA256={ROM_SHA} STATE_SHA256={STATE_SHA}\n")
        output.write("# Source setup: target slot 6, caster slot 0, physical/mental 3/6, seeded RAM.\n")
        output.write("# Code-5 marks come from original 0x1597E -> 0x15CC0 scan; RNG records include caller/bound/result/post-seed.\n")
        writer = csv.DictWriter(output, fieldnames=list(rows[0]),
                                delimiter="\t", lineterminator="\n")
        writer.writeheader()
        writer.writerows(rows)
    print(f"captured {len(rows)} cases to {args.out}")
    print(hashlib.sha256(args.out.read_bytes()).hexdigest())
    capture.close()


def scan_only(rom_path: Path, state_path: Path, state: bytes,
              candidates, out_path: Path) -> None:
    scan = Capture(rom_path, state, actor_scan_hook=True,
                   state_path=state_path)
    try:
        slots = {0, 6} | {int(item["slot"]) for item in candidates}
        scan.reset(slots)
        scan.caster(5)
        source = scan.target(6, 5, marker=0)
        scan.ww(source, 0)
        scan.ww(source + 4, 0)
        for item in candidates:
            slot = int(item["slot"])
            actor = scan.target(slot, 0, marker=0)
            route_cost = int(item["route_cost"])
            # 0x15CC0 returns max(dx,dy)+ceil(min(dx,dy)/2). A two-unit
            # horizontal delta avoids its zero-short-axis action-start arm.
            scan.ww(actor, 2)
            scan.ww(actor + 4, route_cost - 1)
        scan.wl(0xFF1F04, scan.record(0))
        scan.wl(0xFF1F08, source)
        scan.wb(0xFF1F00, 1)
        scan.frame()
        marks = {
            str(int(item["slot"])):
                scan.b(scan.record(int(item["slot"])) + 0xDE) == 5
            for item in candidates
        }
        out_path.write_text(json.dumps(marks, sort_keys=True), encoding="utf-8")
    finally:
        scan.close()


def source_scan_marks(rom_path: Path, state_path: Path | None, state: bytes,
                      candidates) -> dict[int, bool]:
    temporary_state = None
    if state_path is None:
        with tempfile.NamedTemporaryFile(suffix=".state", delete=False) as f:
            f.write(state)
            temporary_state = Path(f.name)
        state_path = temporary_state
    with tempfile.NamedTemporaryFile(suffix=".json", delete=False) as f:
        scan_output = Path(f.name)
    try:
        command = [sys.executable, str(Path(__file__).resolve()),
                   "--scan-only", "--rom", str(rom_path),
                   "--state", str(state_path), "--scan-spec",
                   json.dumps(candidates, separators=(",", ":")),
                   "--scan-out", str(scan_output)]
        result = subprocess.run(command, check=False, capture_output=True,
                                text=True, env=os.environ.copy())
        if result.returncode != 0:
            raise RuntimeError("source spread scan failed:\n" + result.stderr)
        marks = json.loads(scan_output.read_text(encoding="utf-8"))
        return {int(slot): bool(marked) for slot, marked in marks.items()}
    finally:
        scan_output.unlink(missing_ok=True)
        if temporary_state is not None:
            temporary_state.unlink(missing_ok=True)


if __name__ == "__main__":
    main()
