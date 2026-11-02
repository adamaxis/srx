#!/usr/bin/env python3
"""Reproduce the A3 headless emulator combat capture (US ROM, Genesis Plus GX).

Reconstructed from the A3 session's driver. Builds a temporary ROM copy with
two hooks placed in the ROM's 0xFF padding (code under test is unmodified):
  * 0x0D3A (bounded RNG) -> logger at 0x1FFE36 that re-executes the original
    `bsr 0x0D44; mulu d1,d0; swap d0` and appends (caller, bound, value, seed)
    10-byte records at the pointer held in FF1FF0.
  * 0xA8EA (actor service) -> hook at 0x1FFE80: when FF1F00 is set it calls the
    original 0x55640 attack setup and 0x55948 hit/damage with the actor,
    projectile and target records in FF1F04/FF1F08/FF1F0C, then resumes the
    original `jsr 0x13874; jmp 0xA8F0`.
Each seed loads the savestate, seeds FFF9F8, places the two source records and
host-steps one action per emulated frame in ascending source slot order (0xA8EA
order). It measures numeric firearm resolution, not natural AI/action timing.

Outputs the raw capture (--raw) and the compact capture (--out) consumed by
build_a3_emulator_fixture.py. Headless only; needs GPGX_CORE and retro.py.
"""
import argparse, csv, ctypes as C, hashlib, os, sys, tempfile
from collections import Counter
from pathlib import Path

ROM_SHA = "92a614491fc3ea12a6df32459cce08b48574b7b814e244c07eb860dc5c6edb7e"
STATE_SHA = "b9bc8cfbe6341aa93be083c82706977c374e302e4ef01fee42760842eb245019"
LOGGER = bytes.fromhex(
    "4eb900000d44c0c1484048c040e72f012f022f0824 2f00 2620 7c00ff1ff0 2050 20c2 30c1 30c0 343900fff9f8 30c2 23c800ff1ff0 205f 241f 221f 46df 4e75".replace(" ", ""))
HOOK = bytes.fromhex(
    "4a3900ff1f00672e423900ff1f00207900ff1f04227900ff1f0870004eb900055640207900ff1f08227900ff1f0c4eb9000559484e754eb9000138744ef90000a8f0")


def build_harness(rom: bytes) -> bytes:
    b = bytearray(rom)
    for lo, n in ((0x1FFE36, len(LOGGER)), (0x1FFE80, len(HOOK))):
        if set(b[lo:lo + n]) != {0xFF}:
            sys.exit(f"hook area 0x{lo:X} is not ROM padding")
    if b[0xD3A:0xD40] != bytes.fromhex("6108c0c14840") or b[0xA8EA:0xA8F0] != bytes.fromhex("4eb900013874"):
        sys.exit("unexpected original bytes at 0x0D3A/0xA8EA")
    b[0xD3A:0xD40] = bytes.fromhex("4ef9001ffe36")
    b[0x1FFE36:0x1FFE36 + len(LOGGER)] = LOGGER
    b[0xA8EA:0xA8F0] = bytes.fromhex("4ef9001ffe80")
    b[0x1FFE80:0x1FFE80 + len(HOOK)] = HOOK
    return bytes(b)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--rom", required=True)
    ap.add_argument("--state", required=True, help="field savestate (A3 used emu/doors/hw2.state)")
    ap.add_argument("--emu-dir", required=True, help="directory containing retro.py (Emu)")
    ap.add_argument("--raw", required=True)
    ap.add_argument("--out", required=True)
    ap.add_argument("--seeds", type=int, default=64)
    a = ap.parse_args()
    os.environ.setdefault("QT_QPA_PLATFORM", "offscreen"); os.environ.setdefault("SRX_HEADLESS", "1"); os.environ.setdefault("SDL_AUDIODRIVER", "dummy")
    rom = Path(a.rom).read_bytes(); state = Path(a.state).read_bytes()
    if hashlib.sha256(rom).hexdigest() != ROM_SHA: sys.exit("ROM SHA-256 mismatch")
    if hashlib.sha256(state).hexdigest() != STATE_SHA: print("warning: savestate SHA differs from A3's", file=sys.stderr)
    sys.path.insert(0, a.emu_dir); from retro import Emu
    tmp = tempfile.NamedTemporaryFile(suffix=".bin", delete=False); tmp.write(build_harness(rom)); tmp.close()
    e = Emu(tmp.name); e.run(); raw = C.cast(e.ram_ptr, C.POINTER(C.c_uint8))
    def b(x): return raw[(x & 0xffff) ^ 1]
    def wb(x, v): raw[(x & 0xffff) ^ 1] = v & 255
    def w(x): return (b(x) << 8) | b(x + 1)
    def ww(x, v): wb(x, v >> 8); wb(x + 1, v)
    def l(x): return (w(x) << 16) | w(x + 2)
    def wl(x, v): ww(x, v >> 16); ww(x + 2, v)
    def copy_record(live, src, n=0x100):
        for i, v in enumerate(rom[src:src + n]): wb(live + i, v)
    def world_record(live, src, slot):
        # Field copies of the source world-actor initializer.
        copy_record(live, 0x7361A)
        for i in range(2): wb(live + 0xE + i, rom[src + i]); wb(live + 0x2C + i, rom[src + 2 + i])
        for i in range(4): wb(live + 0x4C + i, rom[src + 4 + i])
        wb(live + 0x56, rom[src + 8]); wb(live + 0x57, rom[src + 9])
        for i in range(0x0C): wb(live + 0x58 + i, rom[src + 0x0A + i])
        for i in range(2): wb(live + 0x64 + i, rom[src + 0x16 + i])
        for i in range(0x10): wb(live + 0x66 + i, rom[src + 0x18 + i])
        for i in range(0x18): wb(live + 0x76 + i, rom[src + 0x28 + i])
        wb(live + 0x9A, rom[src + 0x41]); wb(live + 0x9B, rom[src + 0x42]); wb(live + 0x9C, rom[src + 0x43])
        wb(live + 0x90, rom[src + 0x44]); wb(live + 0x91, rom[src + 0x45]); wb(live + 0x8F, rom[src + 0x48]); wb(live + 0x9D, rom[src + 0x49])
        ww(live + 0xA0, (rom[src + 0x46] << 8) | rom[src + 0x47]); ww(live + 0x3E, slot); wb(live + 0x10, 0x25); wl(live + 0x3A, 0x0000B574); wl(live + 0xA2, 0)
    def setup(kind, seed):
        e.load_state(state)
        A, B = 0xFF3000, 0xFF3100
        if kind == "party_enemy":
            slots, ids = (0, 6), ("decker_0x72A1A", "weenr2_0x1D66A4")
            copy_record(A, 0x72A1A); ww(A + 0x3E, 0); world_record(B, 0x1D66A4, 6)
        else:
            slots, ids = (6, 7), ("weenr2_0x1D66A4", "yakuza_weenr_0x1D66EE")
            world_record(A, 0x1D66A4, 6); world_record(B, 0x1D66EE, 7)
        copy_record(0xFF3200, 0x7361A)
        for o in (0x10, 0x11, 0x56, 0x57): wb(0xFF3200 + o, 0)
        ww(0xFFF9F8, seed); wl(0xFF1FF0, 0xFF5000); ww(0xFF1F00, 0)
        return A, B, slots, ids
    def action(kind, seed, no, turn, A, B, slots, ids):
        atk, tgt = (A, B) if turn == 0 else (B, A)
        aslot, tslot = slots if turn == 0 else (slots[1], slots[0])
        code = b(atk + 0x57)
        wb(0xFF3200 + 0x56, b(atk + 0x56)); wb(0xFF3200 + 0x57, code); wb(0xFF3200 + 0x9F, 0)
        pre = w(0xFFF9F8); before = (b(tgt + 0x4C), b(tgt + 0x9C)); p0 = l(0xFF1FF0)
        wl(0xFF1F04, atk); wl(0xFF1F08, 0xFF3200); wl(0xFF1F0C, tgt); wb(0xFF1F00, 1)
        e.pressed = set(); e.run()
        p1 = l(0xFF1FF0)
        rolls = [(l(0xFF0000 + o), w(0xFF0004 + o), w(0xFF0006 + o), w(0xFF0008 + o)) for o in range(p0 & 0xFFFF, p1 & 0xFFFF, 10)]
        att = [r for r in rolls if r[0] == 0x556AA]; dfn = [r for r in rolls if r[0] == 0x559CE]
        other = [r for r in rolls if r[0] not in (0x556AA, 0x559CE)]
        if other: raise RuntimeError(f"unclassified RNG caller(s): {other[:3]}")
        post = (b(tgt + 0x4C), b(tgt + 0x9C))
        return {"mode": kind, "seed": seed, "action": no, "order": f"{slots[0]}>{slots[1]}", "slot": aslot,
                "actor": ids[turn], "target_slot": tslot, "item": code, "seed_before": pre,
                "attack_rolls": ",".join(f"{r[2]+1}@{r[3]:04X}" for r in att), "attack_successes": b(0xFF3200 + 0x9F),
                "defense_rolls": ",".join(f"{r[2]+1}@{r[3]:04X}" for r in dfn), "defense_successes": b(0xFFF0DD),
                "damage": before[0] - post[0], "physical_before": before[0], "physical_after": post[0],
                "mental_before": before[1], "mental_after": post[1], "seed_after": w(0xFFF9F8), "roll_count": len(rolls)}
    rows = []
    for kind in ("party_enemy", "enemy_enemy"):
        wins = Counter()
        for seed in range(1, a.seeds + 1):
            A, B, slots, ids = setup(kind, seed); n = 0
            for rnd in range(1, 41):
                for turn in range(2):
                    if b(A + 0x4C) == 0 or b(B + 0x4C) == 0: break
                    r = action(kind, seed, n + 1, turn, A, B, slots, ids); r["round"] = rnd; rows.append(r); n += 1
                if b(A + 0x4C) == 0 or b(B + 0x4C) == 0: break
            wins["draw" if b(A + 0x4C) and b(B + 0x4C) else ("A" if b(A + 0x4C) else "B")] += 1
        print(kind, dict(wins), sum(1 for r in rows if r["mode"] == kind), "actions", file=sys.stderr)
    os.unlink(tmp.name)
    fields = ["mode","seed","round","action","order","slot","actor","target_slot","item","seed_before","attack_rolls","attack_successes","defense_rolls","defense_successes","damage","physical_before","physical_after","mental_before","mental_after","seed_after","roll_count"]
    with open(a.raw, "w", newline="") as f:
        f.write("# US Genesis ROM emulator capture; source combat functions 0x55640/0x55948; RNG calls from 0x0DBA via 0x0D3A.\n")
        f.write("# order is 0xA8EA source-record service order; no bounded initiative draw exists. One action runs per headless emulated frame.\n")
        wr = csv.DictWriter(f, fieldnames=fields, delimiter="\t", lineterminator="\n"); wr.writeheader(); wr.writerows(rows)
    with open(a.out, "w") as f:
        f.write("# A3 headless US-ROM combat capture; Genesis Plus GX; temporary logger at 0x0D3A; actual ROM routines 0x55640/0x55948.\n")
        f.write("# Source pairs: party slots 0/6 = Joshua Decker 0x72A1A / Weenr2 0x1D66A4; enemy slots 6/7 = Weenr2 0x1D66A4 / Yakuza Weenr 0x1D66EE.\n")
        f.write("# Actions are host-stepped in ascending source-record slot order; this measures weapon numeric resolution, not natural AI/action-delay timing.\n")
        f.write("mode\tseed\taction\tactor_to_target_slot\titem_code\tseed_before\tattack_faces\tattack_successes\tdefense_faces\tdefense_successes\tdamage\tphysical_before\tphysical_after\tmental_before\tmental_after\tseed_after\n")
        for r in rows:
            af = "".join(x.split("@", 1)[0] for x in r["attack_rolls"].split(",") if x)
            df = "".join(x.split("@", 1)[0] for x in r["defense_rolls"].split(",") if x)
            f.write("\t".join(("P" if r["mode"] == "party_enemy" else "E", str(r["seed"]), str(r["action"]), f"{r['slot']}>{r['target_slot']}",
                               format(r["item"], "02X"), format(r["seed_before"], "04X"), af, str(r["attack_successes"]), df,
                               str(r["defense_successes"]), str(r["damage"]), str(r["physical_before"]), str(r["physical_after"]),
                               str(r["mental_before"]), str(r["mental_after"]), format(r["seed_after"], "04X"))) + "\n")
    for p in (a.raw, a.out): print(hashlib.sha256(Path(p).read_bytes()).hexdigest(), p)


if __name__ == "__main__":
    main()
