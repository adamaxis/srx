# Luna handoff — 2026-11-18 (port-fixes @ e3e2ec4a)

Grok Bot is going offline for a few days (usage limit). This is your slate and your method. Work in this order, one item per run, and leave each item either fully fixed with tests or fully diagnosed in `out/DIAG.md`.

## 1. Project direction (read first)

- Goal: a faithful PC port of Sega Genesis *Shadowrun*. **The ROM (`/workspace/shadowrun/rom.bin`) is the spec.** All behavior, formulas, text bytes, tables and timing come from decoded ROM routines. Never "improve" the game and never guess.
- Architecture: C++ is the skeleton only (storage, RNG, collision, rendering, audio, bindings). Gameplay and engine logic lives in Lua (`decompilation/gameplay/scripts/`) so it's easy to modify.
- Code style: named enums and constants (`ActorFlag.PEDESTRIAN`), never raw hex in logic; raw values only in comments. Combine related ROM tables into one class or module. Abstract where possible.
- **State rule (user, 2026-11-17):** dialogue, building interiors and menus are discrete states pushed on top of the field state. Leaving one must restore the exact prior state (scene, music track, pause and input handlers, camera) through ONE generic save/restore. Never write per-choice reset chains ("we chose X, so go outside, play Downtown music..."). Any fix touching these exits must follow this and add a test that the exit restores the pre-entry snapshot.
- Items, stats and screens: decode every item stat from the ROM (don't trust `items.lua`). Menu work lists every button, its position, what's shown, animations, and how sliders, bars and posture feed the real math.

## 2. Method for every task

1. **Reproduce headless** first: `QT_QPA_PLATFORM=offscreen SRX_HEADLESS=1 SDL_AUDIODRIVER=dummy ALSOFT_DRIVERS=null`. Never open a window or real audio on the user's PC. Write down the exact repro.
2. **Find and decode the ROM routine** (`/workspace/m68kdis.py`) into `out/DIAG.md`, with addresses and quoted bytes, **before** coding.
3. **Fix to match the ROM exactly**, following section 1.
4. **Add a test that fails on the old tip and passes after.** Never weaken, skip or delete an existing test. A map-coverage row that only passed because of ROM-wrong behavior may be moved to PRESENT-UNTESTED with a ROM citation (see f14636c4, 9a189a5d).
5. **Rebuild with `-j2`** (`. /workspace/tcenv.sh; cmake --build BUILD -j2`). The box has 15 GB RAM and several builds at once hit OOM. Disk is about 5 GB free, so delete your own build dirs once a fix is pushed.
6. **Full ctest:** `ctest -j6 --timeout 900`, output saved to `out/ctest_full.log`. Baseline is exactly 5 failures: gameplay_lua_suites (equiv_units), rom_coverage, rom_coverage_baseline_semantics, rom_byte_reachability, b3_outside_range_triage_check. frame_debug_server and aircraft_tail_rotor can flake; rerun them alone.
7. **Write `out/STATUS.md`**: what's fixed, files, tests, ROM addresses, repro. End it with `DONE`. If a bug turns out large, ship what's finished and leave the rest diagnosed.
8. **Git rules:** only branch is `port-fixes`; never touch `main` or `sr-refactor`. No force push, no history rewrite, preserve all stashes, WIP and `refs/backup/*`. One logical fix per commit. Don't push incomplete work or regressions. If the ROM shows the reported "bug" is original behavior, say so in STATUS.md with the addresses instead of changing it.

## 3. In-flight work (uncommitted in worktrees, finish first)

| Run | Worktree | What |
|---|---|---|
| pt3companions | /workspace/srx_cab | Hireable runners have no portrait and wrong dialogue; also re-offer "What's your story, chummer?" |
| pt4edge | /workspace/srx_mapcov | Renraku Arcology → Downtown via side edge: runner walks over buildings/garden and can't get back. May be caused by 52a0f262 (edge exit holds the scene until loader 0x167AE/0x2BD2 commits); check collision map and exit tables swap on commit. Also the cab transition after picking a destination is wrong; ROM goes straight to black (taxi clears FFDF40 per 0x136E4). |
| pt4motel | /workspace/srx_stoker | Coffin motel: check in → "never mind" gives a dialogue error, kicks the runner outside, music silent (SFX still play), pause menu dead. Fix per the state rule; list other dialogue exits with per-choice resets. |
| pt4combat | /workspace/srx_fleeinvis | Vs I-fivers: enemies stop shooting after a while and player shots never hit. Trace ROM to-hit and enemy fire state. |

Full briefs are in `briefs_2026-11-18/` next to this file (WT/BUILD/TIP are placeholders).

## 4. Next slate (priority order)

1. **"Slot off frag face" error:** replying that in a random event pops an error dialog window. Find the ROM response routine and fix the handler (playtest 4, #5).
2. **Abandoned-building encounters:** user saw no ghoul/enemy encounters in an abandoned building. Headless spawn in scenes 50–53 works, so find which building and whether the encounter roll ever fires during normal play (timer, entry flags). Waiting on the user's repro.
3. **Full headless walkthrough test, slices 2–8** (slice 1, New Game → Stoker, is in d0dd9aff). Briefs `run_walkthrough_2..8.md`:
   - 2: save round-trip of every story word, plus checkpoint fixtures
   - 3: Salish (FFFC1E b2–b4)
   - 4: Spirit Eyes (b5–b9; expect to stop at the cave)
   - 5: Seattle/Stark (b13, b23–b25, b29, b14)
   - 6: Ritual + Sinsearach (b17–b20, b30, cmd 0x34)
   - 7: Renraku (b11 Mako, b15, b16 Ito, b12, b26–b28)
   - 8: Vigore + ending (b21, b22, tomb gate, credits)
   Each slice drives the real game headless and fails on the first blocker; fix blockers as separate commits.
4. **Remaining cave map-coverage rows** (cave route failures such as `cave-room00-south-r14c0fe`).
5. **About 280 dialogue pages not yet ported**; port from ROM bytes, exact text.
6. **Hurt pedestrians flee (#48)**, then **ROM-driven Ricky hire (#54)**.

## 5. Verified as ROM behavior (don't "fix")

- New decks start at MPCP 0 with zeroed stats; repair to MPCP 4 costs $1,600 (0x1AB190, 0x5787C). Jack-in checks deck, then MPCP, then datajack (0xE40C..0xE488).
- No same-runner inventory reorder exists; only Equip/Unequip, Discard, Trade, Cancel (0x10D30..0x10F4E, Trade 0x1288E..0x12A4E).
- Stage-2 intro music was previously judged silent because its Timer-A DAC writes were not counted; the paired US-ROM trace now shows register `0x2A` data and `0x2B` DAC enables, and event `0x11` is mapped to `audio.intro.title.stage-2`.

## 6. Pushed 2026-11-17/18

f14636c4 Roscoe datafile shop · 841e9b0c Hollywood gate guards · 52a0f262 corpse depth + edge-exit black screen · 272d26fb intro-skip music · 3c88fc19 enemy drops/pickups · 51da4e09 cave room seeding · eaaebce7 Stoker deck purchase · e3e2ec4a doctor/patient event menus.

## 7. Waiting on the user

Windows retest of intro audio; which abandoned building had no encounters; whether the build should hard-fail when the ROM is missing.
