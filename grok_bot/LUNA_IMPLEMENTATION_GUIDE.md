# LUNA IMPLEMENTATION GUIDE: bringing srx (adamaxis/srx @ bfa3908) up to date with the Shadowrun RE work

Read this whole file before you touch anything. Do the phases **in order**. Don't skip acceptance checks.

## 0. Ground rules (non-negotiable)
1. **Lua holds every game rule; C++ is the skeleton.** The skeleton covers presenter, input sampling, sound output, SRAM byte I/O, VDP model and bindings. If you're about to write an `if` that decides game behaviour in C++, stop and put it in Lua.
2. **Named enums, no raw hex.** Every constant comes from `enums.lua` (Lua) or `enums.hpp` (C++). Style: `PascalCase.UPPER`. Never hand-edit `enums.hpp`. Edit `enums.lua` and regenerate it:
   `cd refactor && lua5.4 gen_enums_hpp.lua > enums.hpp`
3. **No `QRandomGenerator` anywhere.** There is one ROM RNG word (FFF9F8), reached through `source_random_seed()` (patch 0001). `rg QRandomGenerator qt_runtime/src` must return nothing in game code when you're done.
4. **Always headless** for any automated or debugger run:
   `SRX_HEADLESS=1 QT_QPA_PLATFORM=offscreen nice -n 10 <exe> --headless`
   No windows, no focus stealing, no audio device. See docs/23 §9.
5. **Keep ROM timing exact.** Never add or remove a frame wait unless the doc gives a ROM address for it.
6. **Never guess an [H] item** (§13). If a step needs one, leave a `-- TODO [H] <doc ref>` and move on.
7. **Never push or force-push**, and don't rewrite history. Work on a local branch and commit per phase.

## 1. What is verified and what is not
- [C] Every Lua test listed below passes on our box (results in §12).
- [C] RNG patches 0001, 0003, 0004, 0005 and 0006 compile with `-Wall -Wextra -Werror`, and their srx tests pass. All 288 srx tests give identical results before and after.
- **[NOT VERIFIED] Anything that needs Qt.** Our box has no Qt6, so the following were never compiled or run:
  - patch 0002 (FieldSceneRuntime)
  - both `*.sketch.cpp` files
  - `out/field_scene_runtime.record.patch`
  - the real headless host
  - every in-game run
  You are the first to build these. Expect compile errors, and fix them without moving rules into C++.
- The srx line numbers below are for **bfa3908**. If upstream moved, re-find the lines by the quoted code, never by number alone.

## 2. Paths used in this guide
- `SR` = `grok_bot/`, the imported plan and reference material.
- `SRX` = the current repository root (`C:\Github\shadowrun`). Do not check out the historical upstream commit or replace the current branch.
- `QT` = `SRX/qt_runtime`, the active Qt runtime under the root Ghidra conversion project.
- `FSR` = `QT/src/field/field_scene_runtime.cpp`.
- `SCRIPTS` = `SRX/gameplay/scripts`, the active Lua content directory.
- `ROM` = `SRX/input/shadowrun_genesis_us.bin`; Ghidra source exports are under `SRX/export/`.
- `SR/refactor`, `SR/tests`, and `SR/tools` are plan/reference copies. Before copying a file, inspect the active destination and merge only behavior that is still missing.
- **C++ include dir:** `QT/src/srx_refactor/`. The runtime include list is in `QT/CMakeLists.txt` (`SHADOWRUN_GHIDRA_QT_RUNTIME_INCLUDE_DIRS`).

## Phase dependency graph
```
1 RNG ──► 2 enums ──► 3 records ──► 4 actor AI ──► 5 menu ──► 6 Matrix rules
                         │                         │
                         └────────► 7 save ◄───────┘
2 ──► 8 rendering (independent of 4-7, needs only 2)
1+2 ──► 9 debugger/headless/heartbeat (9 needs 4 for the heartbeat hook)
2+5 ──► 10 story flags / FFDD64 rule
```

---
## Phase 1: RNG patches 0001-0006 (do first; everything random depends on it)
**Source:** `SR/out/patches/0001..0006*.patch` and `SR/out/patches/README.md`. Background: `SR/docs/24_rng_audit.md` (R1-R10).
1. The current checkout and branch are authoritative. Do not reset, replace, or branch from the historical srx commit `bfa3908`.
2. Dry-run, then apply, **in order**:
   `for p in SR/out/patches/000*.patch; do patch -p1 --dry-run < $p || break; done`
   Then the same loop without `--dry-run`.
3. What each patch does:
   - 0001: the shared `source_random_seed()`, cold boot with d1 = 0x0000FFFF, R5 (a negative dice count rolls one die), and the observer hook.
   - 0002: R1. `field_random_seed_` becomes a reference to the shared word. **Deletes the QRandomGenerator seeding at FSR.cpp:6830.**
   - 0003: R2, the title state.
   - 0004: R3/R4, effect jitter.
   - 0005: R6, the node-action gate becomes min(pool, Computer) + Computer.
   - 0006: R7, comment only.
4. Build and test: configure with the repo's CMakePresets, build, then run ctest **headless**: `SRX_HEADLESS=1 QT_QPA_PLATFORM=offscreen ctest --output-on-failure`.
   Expected: the same pass/fail set as unpatched bfa3908, with 3 tests that pre-existingly don't build and 1 that aborts in both builds. The title_loop and matrix-gate tests are changed by the patches and must pass.
5. Acceptance:
   - `rg -n QRandomGenerator QT/src` → no hits in game code.
   - A cold boot from zero RAM gives seed 0xFFFF.
   - At the title, after the boot intro, the seed is **0xD29F** (the intro 0x5D746 draws 224 values per frame for 0x151 frames; docs 24/25).
6. Pitfalls:
   - Do not "fix" the 0/negative-dice rule; it is ROM behaviour.
   - 0002 was never compiled. If `FieldSceneRuntime` gets copied anywhere, the reference member breaks copy-assign. We found no copies with rg, but the compiler decides.
   - Do not seed from time, ever.

## Phase 2: enums (`enums.lua` → `enums.hpp`)
1. Copy `SR/refactor/enums.lua` to `SCRIPTS/enums.lua`, and `SR/refactor/gen_enums_hpp.lua` to `SCRIPTS/gen_enums_hpp.lua`.
2. Copy `SR/refactor/lib/int68k.lua` to `SCRIPTS/lib/int68k.lua`. It holds the signed-byte/word helpers; don't write your own `& 0xff` sign hacks.
3. Generate the header: `cd SCRIPTS && lua5.4 gen_enums_hpp.lua > ../../qt_runtime/src/srx_refactor/enums.hpp`.
4. Acceptance:
   - `echo 'int main(){}' > /tmp/t.cpp && g++ -std=c++20 -Wall -Werror -include enums.hpp -fsyntax-only /tmp/t.cpp` passes.
   - The generated file matches `SR/refactor/enums.hpp` byte for byte.
5. Pitfalls:
   - Enum **values** equal the ROM values. Renames (e.g. `ActorServiceFlag.*` → `ActorStatus.*`) change names only (refactor/MIGRATION.md "Renames applied").
   - Don't renumber anything.

## Phase 3: records (items / spells / accessories / actor_profiles / weapons)
1. Copy `SR/refactor/data/{items,spells,accessories,actor_profiles,weapons}.lua` to `SCRIPTS/data/`.
2. Copy the generators `SR/refactor/gen_records_hpp.lua` and `SR/refactor/gen_weapons_hpp.lua`, then run:
   `lua5.4 gen_records_hpp.lua > .../srx_refactor/records.hpp`
   `lua5.4 gen_weapons_hpp.lua > .../srx_refactor/weapons.hpp`
   Compare the output with `SR/refactor/records.hpp` and `SR/refactor/weapons.hpp`.
3. The data files are generated from the ROM by `SR/tools/gen_records.py`. **Regenerate them; never hand-edit.**
4. C++ only stores and looks records up. Every behaviour method stays in Lua (header of gen_records_hpp.lua).
5. Acceptance:
   - Both headers compile with -Werror.
   - All 50 item prices, magazines and armor values match srx's items.lua (doc 08 diff).
   - Accessory prices come from 0x1AAE07/0x1AAE8C.
6. Pitfall: `attack_delay_scale` and `reload_delay_scale` in srx items.lua have **no ROM source** (doc 02 :81). Don't "port" them as truths.

## Phase 4: actor AI split modules
**Source:** `SR/refactor/actor/{tables,targeting,anim,actions,movement,heartbeat,registry,init}.lua`. Notes: `SR/refactor/MIGRATION.md`. C++ reference: `SR/out/ai_reference.cpp`, `SR/out/ai_bindings.hpp`, `SR/docs/20a_ai_cpp.md`.
1. Copy the modules to `SCRIPTS/actor/`. Point `require("actor_ai")` at `actor.init`. The returned `ActorAI` table and `ActorAI.heartbeat(context)` keep their signatures.
2. Keep the old `SCRIPTS/actor_ai.lua` until the equivalence tests pass, then delete it in a separate commit.
3. Tests (no Qt):
   `LUA_PATH="SR/refactor/?.lua;;" lua5.4 SR/tests/equiv_heartbeat.lua` expects `mismatches=0`.
   `lua5.4 SR/tests/equiv_units.lua` expects `functions=57 calls=17100 mismatches=0`.
   `SR/tests/mutants.sh` expects all 8 planted mutants caught.
   These tests load the original srx actor_ai.lua. Adjust its path at the top of each test if it differs.
4. Stage 3 (later): movement.lua moves behind C++ (`game.movement.*`, per out/ai_bindings.hpp). Only integration, probes and collision go there.
5. Pitfalls:
   - Read registry names **inside functions**, not at load time; otherwise they stay nil.
   - 0x1514E is `fire_shot`, not melee (doc 20a §5).
   - FFDF42 has no ROM writer (20a §7); don't invent one.
   - The 20a §2 steering edge cases (Δ = 0 asymmetry; the sticky x-overshoot flag hides the y check) are ROM behaviour. Keep them.

## Phase 5: menu route stack, Lua screens/rules, bindings, host wiring
**Source:** `SR/refactor/menu/` (`init.lua`, `route.lua`, `screens/*.lua`, `rules/*.lua`, `menu_binding.hpp`, `host/*.sketch.cpp`, `MIGRATION.md`).
1. Copy `SR/refactor/menu/{init,route}.lua`, `screens/` and `rules/` to `SCRIPTS/menu/`. Copy `menu_binding.hpp` to `QT/src/srx_refactor/menu/`.
2. Use `refactor/menu/MIGRATION.md` "Moved blocks" as the edit list. Each FSR/bridge range there is replaced by `route.frame(events)` plus the named Lua screen. The ranges:
   - FSR 10498-11020
   - FSR 5389-5420
   - bridge 172-240
3. **Dead FSR code to delete once the route stack runs:**
   - the twelve child-open booleans and their if-chain (FSR 10510-10945, D1)
   - the selector-suppressed latch (FSR 10599-10602)
   - the ~20-field close/reset block (FSR 10946-10985; the replacement is `host.menu_closed()`)
   - the six FSR wait flags (D10)
   Delete each one only after its screen's tests pass.
4. Wire the hosts:
   - Implement `FsrMenuGame` and the MenuHost following `host/menu_game_fsr.sketch.cpp`. FSR needs `friend class FsrMenuGame;`.
   - Implement MatrixGame following `host/matrix_game_srx.sketch.cpp`. It maps `cyberspace/matrix_node_action_loop_conversion.hpp:165-214` callbacks onto Lua.
   - Both sketches cite srx file:line. **They were never built.**
5. Add the new .cpp files to `SHADOWRUN_GHIDRA_QT_GAME_SOURCES`, then build with -Werror.
6. Tests (no Qt):
   `lua5.4 SR/tests/menu_tests.lua` expects `28/28 passed`.
   `lua5.4 SR/tests/submenu_tests.lua` expects `177/177 passed`.
7. Acceptance, in game, headless with the debugger:
   - Opening the Command page plays sound 0x25.
   - Start in posture exits the whole menu (D4).
   - A full re-init resets to page 6 (D6).
8. Pitfalls:
   - `legacy.lua` is transitional. Port screens off it; don't add new legacy screens.
   - Window plane per screen (D9): only Attributes/Skills and Cyberdeck hide the sidebar.

## Phase 6: Matrix rules
1. The rules are in `SCRIPTS/menu/rules/{matrix,matrix_combat,matrix_move,matrix_programs,node_actions,deck,datastore_special}.lua`, copied in Phase 5.
2. Bind them through MatrixGame (Phase 5 step 4). srx's gate body (`matrix_node_action_loop_conversion.cpp:205-246`) is **bypassed**: the callback returns the Lua verdict. The gate is min, not max (D38/R6).
3. Test: `lua5.4 SR/tests/matrix_actions_tests.lua` expects every line `ok`, ending with `ralert ok` (26 checks plus gate and ralert).
4. Pitfalls:
   - Free-memory compares are 16-bit signed (D28).
   - The decker test is min(pool, rating) + rating (D32).
   - 0x1A154 crashes the IC (D34).
   - Transfer Data searches before it asks (D35).

## Phase 7: save format
1. `SCRIPTS/menu/rules/save.lua` and `screens/save_load.lua` (copied in Phase 5) own the format. C++ only moves SRAM bytes: byte n lives at 0x200000 + 2n + 1, 2 slots × 2 copies of 0x800. See doc 07.
2. Edit: replace srx's save path with a host service that reads and writes raw bytes. Lua builds and parses the image (FFF1E8 layout, magic "GAME" at +4).
3. Deviation D29: the ROM repairs slot 0's copies whatever slot is loaded. Our Lua repairs the **loaded** slot. Set `ROM_REPAIR` to get the ROM bug back for capture comparisons.
4. Acceptance:
   - A `.srm` exported by an emulator imports, and re-exports byte-identical except for the repaired copy.
   - Covered by submenu_tests.

## Phase 8: rendering fixes R1-R11, smallest first (doc 21 §2, §4)
Do them in this order. Each is its own commit with a capture diff.
1. **R1 (one line, FSR:12000-12017):** paint actors in reverse of the descending-depth sort. SAT index 0 is on top, so DOWN/DEAD actors go under the living.
2. **R4 (FSR:14983-14984, 7243-7247):** sprites at x ≥ 272 must draw over the HUD window. Window tiles have priority 0.
3. **R5 (FSR:14224-14233, elevator_selector_runtime.cpp:198-203, FSR:10207):** replace the black alpha overlay with a per-channel saturating CRAM fade (0x1088 out, 0x10D6 in).
4. **R3 (FSR:7744 vs 6886):** `field_terrain_render_dirty_` is set but never read. Re-render the 16×16 cell when `layer_a` changes.
5. **R2/R9:** replace the flattened map with 3 name tables plus `VdpModel`.
6. **R7/R6:** SAT pieces with the 20-sprite/320 px per line and 80 total limits, and the VBlank latch (one frame of lag).
- **R10 is already correct; don't change it.** R8 and R11 are **[H]; don't touch them.**

## Phase 9: debugger, headless mode, heartbeat recorder
1. Copy `SR/refactor/debugger/srxdbg/*.hpp` (core, debugger, headless, server, source_rng, vdp) to `QT/src/srx_refactor/srxdbg/`. Copy `cli/srxdbg.py` to `SRX/tools/`.
2. Host startup: call `srxdbg::headless::parse/apply` **before** constructing `QGuiApplication`. When headless, skip `MainWindow::show()` and the audio device open.
3. Replace `on_host_frame_timeout`/`advance_one_source_frame` (window.hpp:968/979) with `Debugger::tick`, following the docs/23 §7 table.
   - Remove the 25 `set_source_random_seed_for_dump` seed sites in qa_main.cpp in favour of the SeedSet header.
   - Follow the doc 22 §6 order.
4. Heartbeat recorder:
   - Integrate `SR/refactor/tools/heartbeat_recorder.lua` under `SCRIPTS/tools/` only if the active project does not already provide the recorder.
   - Integrate `SR/out/heartbeat_record_hook.hpp` under `QT/src/field/` only if still needed; map the historical patch paths to current files before use. **Not compiled.**
   - Run with `SRX_HEARTBEAT_RECORD=/tmp/play.rec`, then `LUA_PATH="SR/refactor/?.lua;SR/refactor/tools/?.lua;;" lua5.4 SR/tests/replay_heartbeat.lua /tmp/play.rec` and expect 0 mismatches and 0 divergence. Without a .rec argument the script errors; that is expected.
5. No-Qt tests:
   - `g++ -std=c++17 -Wall -Wextra -Werror -pthread SR/refactor/debugger/tests/mock_game_test.cpp` should report 30/30.
   - Build and run `source_rng_test.cpp` and `headless_host_stub.cpp` the same way.

## Phase 10: story flags and the FFDD64 rule
1. `E.StoryFlag` in enums.lua names FFFC1E bits 0-31 (doc 14 §3.2). Use the names; never write `bit 0x1C`.
2. **FFDD64 rule [C], must be ported exactly:**
   - The trigger loop 0x53A78 writes trigger index i to FFDD64 before calling handler i.
   - The generic setter 0x2196C sets FFFC1E bit FFDD64.
   - Dialogue commands 0x15-0x1D and cmd05 call the setter, so they set **the bit of the trigger that opened the conversation**.
   - Some handlers first store an immediate (5, 8, 0x11, 0xD, 0x14, 0x15, 0x19, 0x1A, 0x1B, 0x1C).
   - Implement this as Lua `story.current_trigger` plus `story.set_current()`, and keep the value for the whole dialogue. Otherwise bits 0-5 are never set.
3. **Loop quirk, kept:** the loop reloads its index from FFDD64 after each handler. A handler that writes FFDD64 and returns d6 ≥ 0 moves the loop.
4. Trigger table 0x15FD5C: live indices are 0-4, 10, 12, 14, 15, 18, 19, 21 and 22; the rest are no-ops (0x49A0).
5. FFFC1E is cleared whole at new game (0x818). Bit 18 is never cleared.
6. Bits 11 and 16 are set from encounter-cleared callbacks (0x53FFC, 0x54286), not from dialogue. Bit 12 is set from tile code 0x1A in scene 0x28.
7. Tool: `python3 SR/tools/fc1e_sites.py` regenerates `out/fc1e_sites.tsv` (run it from SR with rom.bin and disasm present).

---
## 11. Behaviour-fix checklist D1-D41 (tick each when ported; full text in refactor/menu/MIGRATION.md)
- [ ] **D1** Twelve child-open booleans → one explicit route stack
- [ ] **D2** Labels in Lua, rules in C++ → rules in Lua, labels and painting in C++
- [ ] **D3** Rebuilds use a `build(full)` call, not close-then-reopen
- [ ] **D4** Behaviour fix [C]: Start inside the posture focus exits the whole Command menu
- [ ] **D5** Behaviour fix [C]: the C button calls 0x391A (leader reselect) and then the 0x10D90 refresh, which plays 0x25
- [ ] **D6** Behaviour fix [C]: a full re-init resets the page to 6
- [ ] **D7** Opening plays 0x25 [C] (0x10DC2 → 0x10E2E), and so does every 0x10D90 refresh after a submenu.
- [ ] **D8** Held-direction guard [C]: Up/Down on pages ≥2 with Down/Left/Right *held* (FFDD22 bits 1-3) doesn't open posture but still plays 0x25 and refreshes.
- [ ] **D9** Window plane per screen: the route applies `screen.window` for the top screen, so only Attributes/Skills (and later Cyberdeck) hide the sidebar [C docs/09 §11].
- [ ] **D10** One generic MESSAGE screen (screens/message.lua) replaces the six FSR wait flags (clips, item, first aid, automatic use, notebook, johnsons).
- [ ] **D11** Discard prompt defaults to No and has no Start path, per ROM 0x12694 (FSR MenuConfirmationState could default to Yes) [C].
- [ ] **D12** Item-action children REPLACE the action row, so their result returns straight to the Command page; no "return-to-row" flag.
- [ ] **D13** Attributes/Skills from the Command page is view-only (srx command_view_only); karma upgrades only when opened with args.karma (hotel)
- [ ] **D14** `leader_spell_gate` renamed to `leader_archetype`: +0x9B is the archetype (< 2 = caster), not a gate.
- [ ] **D15** Pocket Secretary: only A (bit 14) dispatches; B does nothing (FSR treated B like A) [C 0xC2A6]
- [ ] **D16** Magic: the force/posture loop is two stacked screens (MAGIC_FORCE, MAGIC_POSTURE) instead of a mode int.
- [ ] **D17** Clips is ONE-SHOT per ROM 0x11B58: every outcome prints its text, waits for any edge (0xD94, silent) and returns to the Command page; Cancel / trade-Start print "Function
- [ ] **D18** Item action row 0x117D4: A or B confirm, C no-op, Start silent exit (was: B/Start exit) [C]
- [ ] **D19** Species attribute maxima now cite ROM 0xC22A6 (5x6, indexed +0x9A*6+i at 0xD8A4); a test compares the Lua table to rom.bin [C].
- [ ] **D20** Automatic-use items (0x13, 0x14, 0x16, 0x18..0x2B: fetish, passkey, smart goggles, foci, talisman, spell lock, totems) only print "This item is automatically used." (0xD2
- [ ] **D21** Pocket Secretary row 5 = the debug menu 0x599DA (docs/12), now screens/debug_menu.lua (+ DEBUG_PICK_SYSTEM)
- [ ] **D22** Magic value maths moved to Lua (rules/magic.lua; docs/02 "Magic value maths"): success dice, drain, fetish absorption and magical defense, with all four ROM tables checke
- [ ] **D23** Pick System return path is now ROM-exact (0xE47A..0xE4C6)
- [ ] **D24** Save/Load 0xEFB4 is now a Lua screen (screens/save_load.lua) and replaces legacy()
- [ ] **D25** Cyberdeck 0x18150/0x1822C is now a Lua screen (screens/cyberdeck.lua) with the 4x4 grid, the FFF0B7 mode per button, and the pull-the-plug prompt
- [ ] **D26** The program slot screen 0x19238 and the 0xE2DFE functions (CyberInfo, Storage, Map/Retreat) are now Lua (screens/deck_screens.lua, rules/deck.lua)
- [ ] **D27** Texts the ROM prints with a plain rts (no slots/memory, not on a run, run complete, no datafiles, no retreat node) return {text=} to the grid instead of a MESSAGE wait, m
- [ ] **D28** Free-memory compares use 16-bit signed words (cmp.w/bpl) as the ROM does
- [ ] **D29** Save repair fix: the ROM repairs slot 0's copies whatever slot is loaded (doc 07 §7)
- [ ] **D30** The IC type and rating (0x19B40 / 0x19BF6), alert changes (0x1989A), map reveal (0x533EE), slot cycling (0x19432) and logout timing are now Lua rules (rules/matrix.lua)
- [ ] **D31** The Matrix combat and movement rules are now Lua (rules/matrix_combat.lua, matrix_move.lua): pool split, program rolls, Attack/Slow/Degrade, Smoke/Mirrors/Shield, IC turn
- [ ] **D32** Doc fix: the decker test is min(pool, rating) + rating
- [ ] **D33** Node actions 0x54AEA..0x5561E and the programs Rebound/Medic/Sleaze/Deception/Relocate/Analyze are now Lua (rules/node_actions.lua, matrix_programs.lua, datastore_special
- [ ] **D34** Fix: 0x1A154 crashes the IC (+0x0F = 0)
- [ ] **D35** Doc fix 05 §8.7: Transfer Data searches first (detect = (Int+Computer+Sensor)/3 vs rand(10)+1), then asks whether to download
- [ ] **D36** Analyze (0x56064): net = decker successes - roll(node security, max(Sensor,2)); FFFB30[FFF102] |= min(net,4)<<4
- [ ] **D37** RNG: the ROM has ONE stream (FFF9F8: 0xD44/0xD3A/0xDBA)
- [ ] **D38** Fix vs srx: the node-action gate 0x548BE uses min(pool, Computer) + Computer (srx matrix_node_action_loop_conversion.cpp:209 uses max)
- [ ] **D39** RNG patches 0001-0006 for srx bfa3908 are in out/patches/ (unapplied)
- [ ] **D40** StoryFlag enum (refactor/enums.lua E.StoryFlag), FFFC1E bits 0-31 with [C]/[H]
- [ ] **D41** tools/fc1e_sites.py fixes: (a) FFDD64 imm writes inside a trigger handler override the loop index (bit 8 at 0x53F50, not bit 4); (b) a register reloaded from FFFC34 ends

## 12. ROM quirks: kept vs deviated
**Kept on purpose (ROM-exact; do NOT "fix"):**
- RNG:
  - 0 or negative dice still roll one die.
  - Sixes explode.
  - The TN compare is signed.
  - rand(n) = (raw × n) >> 16.
  - Cold-boot seed 0xFFFF; title seed 0xD29F (D37/D39).
- Free-memory checks are 16-bit signed compares (D28). Matrix gate and decker test use min (D32/D38).
- D8 held-direction guard: plays 0x25 without opening posture.
- Clips is one-shot (D17).
- The discard prompt defaults to No (D11).
- Pocket Secretary: B does nothing (D15).
- The trigger loop reloads FFDD64. Commands 0x15-0x1D set the current trigger's bit (D40/D41).
- Steering: Δ = 0 asymmetry and the sticky x-overshoot flag (doc 20a §2).
- R10 depth forcing by `flags & 0x50`.

**Deliberate deviations (switchable or documented):**
- D29 save repair: repairs the loaded slot, where the ROM repairs slot 0. Flag `ROM_REPAIR` restores the ROM behaviour.
- D1/D3/D10/D12/D16: the route stack replaces flags and mode ints. Behaviour is equal; only the structure changed.
- D27: plain-rts texts return `{text=}` instead of a MESSAGE wait. Timing is identical per MIGRATION.
- Fixes against srx behaviour that **are** the ROM: D4, D5, D6, D11, D13, D15, D18, D34, D35, D38, R1-R7.

## 13. Open [H] items: do NOT guess
- **R8**, camera clamp (width − 272 vs − 320; FFDDD4/FFDDEA). **R11**, the projectile `selector & 3` skip.
- Which scenes use plane-B offsets (0x26E2/0x1AB0). Whether any actor sets +0x2C bit 15. The SAT link terminator. Dynamic H-int.
- StoryFlag names tagged [H name] in enums.lua: bits 0, 1, 2, 10, 13, 17, 19, 24, 25, 28. **Bit 31: no setter found.**
  - cmd1B (bit 17) and cmd1C (bit 13) have no invoking dialogue node in our graph, which may be incomplete.
- Menu: drain-damage tail 0x1046E (stays in the C++ damage service); FFF0B1 meaning inside the Matrix; the selector-suppressed latch removal (verify after the ITEM_ACTION port).
- Actor flag renames assume the host fills +0x9E/+0x10/+0xB0 (refactor/MIGRATION.md Risks).
- Debugger: snapshot cost, Lua state serialisation of route stacks, the audio effect on pacing, a ROM-derived reference renderer (docs/23 §8). The real Qt host window/audio sites are not wired.
- Matrix: alert-band bytes (05:51), persona +0x0C (05:33), pool detail (05:102).
- Everything else listed in `docs/open_hypotheses.md` (249 lines, by doc).

## 14. Test results on our box (2026-10-15, Lua 5.4, no Qt)
| test | result |
|---|---|
| tests/menu_tests.lua | 28/28 passed |
| tests/submenu_tests.lua | 177/177 passed |
| tests/matrix_actions_tests.lua | all ok (last line `ralert ok`) |
| tests/equiv_units.lua | functions=57 calls=17100 mismatches=0 |
| tests/equiv_heartbeat.lua | calls=240000 mismatches=0 (1231 errors raised identically by both) |
| tests/replay_heartbeat.lua | needs a `.rec` argument; no real recording exists (needs Qt) |
| enums.hpp | -Wall -Werror syntax check ok |
| srx tests after patches | 288 built; identical results to base |

## 15. Doc map
- docs/00-25: ROM subsystems.
- docs/coverage.md: what is covered.
- docs/lua_review.md: Lua review.
- docs/srx_project_summary.md: the srx architecture.
- refactor/MIGRATION.md: the actor split.
- refactor/menu/MIGRATION.md: the menu and D-list.
- out/patches/README.md: the RNG patches.
