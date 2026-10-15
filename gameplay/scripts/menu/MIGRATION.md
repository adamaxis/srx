# Menu split: migration notes
The rules move to Lua (route.lua plus screens/*). C++ keeps only menu_binding.hpp: the presenter, input timing, sound output, display blank and window plane.
FSR = srx `decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime.cpp`; bridge = `field_command_overview_bridge.cpp`.
Tests: `lua5.4 tests/menu_tests.lua` (27 checks).

## Moved blocks
| FSR / bridge location | what | new home | status |
|---|---|---|---|
| FSR 10498-10509 | per-frame wait/latch/sample | C++ stays: sample, then `route.frame(events)` | skeleton |
| FSR 10510-10571 | command-view Attributes/Skills child | `screens/attributes.lua` (`Attributes`, view-only); Qt presenter mirrors Lua cursor | **ported for command view** |
| FSR 10572-10576 | clips branch (advance 6049) | legacy CLIPS | port queue |
| FSR 10577-10581 | magic spell interface (advance 5914) | legacy MAGIC | port queue |
| FSR 10582-10598 | automatic_use 6581, first_aid 6597, inventory_trade 6400 | children of ITEM_ACTION (legacy) | port queue |
| FSR 10599-10602 | selector-suppressed latch | dropped: the route stack makes it unnecessary [H, verify after ITEM_ACTION port] | removed |
| FSR 10603-10612 | Cyberware full-screen wait/return | `screens/wait_any.lua` (`FULL`); Qt presenter builds the typed +0x90 rows | **ported** |
| FSR 10613-10625 | magical_defense_wait | `wait_any` (MAGICAL_DEFENSE); MAGIC pushes it once ported | ported module |
| FSR 10626-10692 | posture dialog | `screens/posture.lua` | **ported** (fix D4) |
| FSR 10693-10938 | pocket secretary / dismiss runner | legacy POCKET_SECRETARY (returns `exit_menu` = FFF0CD) | port queue |
| FSR 10939-10945 | inventory_action (6471, discard 6333) | legacy ITEM_ACTION | port queue |
| bridge 196-240 | page/row/A/C/Start priority | `screens/command.lua` `input` | **ported** |
| bridge 172-192 + `advance_command_overview_secondary_page` | 0x11506 row and posture gate | `command.step_row` | **ported** |
| FSR 10946-10985 | close and reset of ~20 child fields | route `EXIT` → `host.menu_closed()` (C++ clears display_service_word, FSR 10957) | **ported** |
| FSR 10986-11020 | A/B dispatch to begin_* (5543-5819) | `command.input` CHILD table (0xD2A0A) | **ported** |
| FSR 5389-5420 | open_command_overview resets | `Route:open` + `command.enter` (new state per open) | **ported** |
| FSR 11697 | seek_command_overview_page (QA) | tests drive `route.frame` directly | QA only |

## Design fixes ("looks dumb", with reasons)
- **D1. Twelve child-open booleans → one explicit route stack.** FSR picks the active child by an if-chain (10510-10945). Nothing stops two flags being set at once. Each child resets by hand in two places (5389 and 10948). The route makes "which screen has input" a single fact and keeps its state per entry.
- **D2. Labels in Lua, rules in C++ → rules in Lua, labels and painting in C++.** This diverges from the srx qt-menu-class-design.md, where the conversion layer owns selections.
- **D3. Rebuilds use a `build(full)` call, not close-then-reopen.** The `child_returned` table copies the ROM tail at 0x10E4E-0x10EC6: page <2 → 0x11592 only; page 3 → full re-init; page 6 → exit or re-init; magic with +0x9B ≤1 → re-init; otherwise 0x10D90 refresh [C].
- **D4. Behaviour fix [C]: Start inside the posture focus exits the whole Command menu.** It sets FFF0B1 (0x11A92), and 0x10E1C exits on it. FSR 10679 only closed the dialog.
- **D5. Behaviour fix [C]: the C button calls 0x391A (leader reselect) and then the 0x10D90 refresh, which plays 0x25.** The bridge (236-240) only moved to page 6; FSR never consumes `mode_action_pending` (grep).
- **D6. Behaviour fix [C]: a full re-init resets the page to 6.** 0x10F50 writes FFF0D0=6 and FFF0D1=0, so returning from Attributes/Skills (or Magic with +0x9B ≤1) lands on Pocket Secretary. The field route adapter exercises this Attributes/Skills return in embedded-runtime and Qt tests.
- **D7. Opening plays 0x25 [C]** (0x10DC2 → 0x10E2E), and so does every 0x10D90 refresh after a submenu.
- **D8. Held-direction guard [C]:** Up/Down on pages ≥2 with Down/Left/Right *held* (FFDD22 bits 1-3) doesn't open posture but still plays 0x25 and refreshes.
- **D9. Window plane per screen:** the route applies `screen.window` for the top screen, so only Attributes/Skills (and later Cyberdeck) hide the sidebar [C docs/09 §11].

- D10 One generic MESSAGE screen (screens/message.lua) replaces the six FSR wait flags (clips, item, first aid, automatic use, notebook, johnsons).
- D11 Discard prompt defaults to No and has no Start path, per ROM 0x12694 (FSR MenuConfirmationState could default to Yes) [C].
- D12 Item-action children REPLACE the action row, so their result returns straight to the Command page; no "return-to-row" flag.
- D13 Attributes/Skills from the Command page is view-only (srx command_view_only); karma upgrades only when opened with args.karma (hotel). Previously one flag in FSR did both.
- D14 `leader_spell_gate` renamed to `leader_archetype`: +0x9B is the archetype (< 2 = caster), not a gate.
- D15 Pocket Secretary: only A (bit 14) dispatches; B does nothing (FSR treated B like A) [C 0xC2A6]. Save/Load + Cyberdeck stay host services (legacy adapter) and report FFF0CD as exit_menu.
- D16 Magic: the force/posture loop is two stacked screens (MAGIC_FORCE, MAGIC_POSTURE) instead of a mode int.

- D17 Clips is ONE-SHOT per ROM 0x11B58: every outcome prints its text, waits for any edge (0xD94, silent) and returns to the Command page; Cancel / trade-Start print "Function Aborted!" (0xD1ED8, new MenuMessage.FUNCTION_ABORTED=21). B and C act like A; the main loop has NO Start check. Reload prompt: default No (0x11D00 clr + 0x127B0 eori), A only, "No" exits silently. srx/FSR kept the menu open and closed on B/Start: replaced [C].
- D18 Item action row 0x117D4: A or B confirm, C no-op, Start silent exit (was: B/Start exit) [C]. First aid 0x1235C: A/B/C confirm, Start silent exit [C].
- D19 Species attribute maxima now cite ROM 0xC22A6 (5x6, indexed +0x9A*6+i at 0xD8A4); a test compares the Lua table to rom.bin [C].
Sound sites for all of these: out/menu_sound_sites.txt (0x25 = move/toggle, 0x24 = confirm; Start paths silent) [C].

- D20 Automatic-use items (0x13, 0x14, 0x16, 0x18..0x2B: fetish, passkey, smart goggles, foci, talisman, spell lock, totems) only print "This item is automatically used." (0xD202F) and wait (0x12168 -> 0x121C0). The menu applies no effect, so nothing is missing from the port [C]. The passive effects are read elsewhere by code; that's outside the menu scope.
- D21 Pocket Secretary row 5 = the debug menu 0x599DA (docs/12), now screens/debug_menu.lua (+ DEBUG_PICK_SYSTEM). It is one-shot (FFF101 is cleared on A). The effects are Lua rules over the party table and named RAM-write services. Pick System → jack-in exits the menu tree [H: exact return path].

- D22 Magic value maths moved to Lua (rules/magic.lua; docs/02 "Magic value maths"): success dice, drain, fetish absorption and magical defense, with all four ROM tables checked against rom.bin. Removed the game.magical_defense() / magic_values() C++ services. The magic screens keep st.values = {success, drain}.
- D23 Pick System return path is now ROM-exact (0xE47A..0xE4C6). After the Matrix, the Pocket Secretary closes and the Command page rebuilds; it does not leave the menu (D21 had guessed "exit"). Sound 0x1A = MenuSound.JACK_IN.

## Timing contract [C unless marked]
- One `route.frame` per VBlank sample.
- Sounds and redraws happen in that same frame. ROM text writes happen during active display, right after the wait.
- A full build = `blank(OFF)`, then FULL, then `blank(ON)`. That gives at least one black frame; the exact count is [H] (it depends on the build length, not measured).

## Port queue (next)
Done: item action, clips, attributes/skills, magic, pocket secretary (tests/submenu_tests.lua 50/50). Left in legacy: SAVE_LOAD, CYBERDECK (host services by design).
Open [H]: drain-damage tail at 0x1046E (kept in the C++ damage service); FFF0B1 meaning inside the Matrix. (Auto-use and row 5 resolved: D20, D21.) (Clip/dismiss/item sounds and the species table are now ROM-traced, D17-D19.)
In-game wiring: host/menu_game_fsr.sketch.cpp (MenuGame/MenuHost over srx FieldSceneRuntime, file:line per service).

Order: ITEM_ACTION (6471, 6333, 6400, 6581, 6597), CLIPS (6049; ROM 0x11B58), POCKET_SECRETARY (C2A6, 0xC376 widget), MAGIC (5914, 0xFBA6), ATTRIBUTES_SKILLS (0xD01C).
Each one replaces its `legacy(...)` line in menu/init.lua.

- D24 Save/Load 0xEFB4 is now a Lua screen (screens/save_load.lua) and replaces legacy(). Gate texts (FFDF40, FFDF84) return to the Pocket Secretary. Start in the menu or slot picker (FFF0CC) returns to the Pocket Secretary without exiting. Every save/load result, including "no game saved here", exits the whole menu (FFF0DC + FFF0CD). The old adapter exited only on a host flag.
- D25 Cyberdeck 0x18150/0x1822C is now a Lua screen (screens/cyberdeck.lua) with the 4x4 grid, the FFF0B7 mode per button, and the pull-the-plug prompt. Dumpshock rules are Lua. Sub-loops 0x19238 and 0xE2DFE stay host services. The "no deck" message is still a Pocket Secretary MESSAGE. Fix vs the old adapter: a failed Willpower roll that the runner survives does NOT log out (0x18598 → 0x1831A).

- D26 The program slot screen 0x19238 and the 0xE2DFE functions (CyberInfo, Storage, Map/Retreat) are now Lua (screens/deck_screens.lua, rules/deck.lua). The host services deck_program_slots/deck_function are gone. C++ keeps only RAM proxies (deck_record/deck_slots + deck_commit), painting, and the map/retreat VDP work.
- D27 Texts the ROM prints with a plain rts (no slots/memory, not on a run, run complete, no datafiles, no retreat node) return {text=} to the grid instead of a MESSAGE wait, matching the ROM. Storage only acts on A and its erase prompt defaults to Yes; both are ROM quirks, kept.
- D28 Free-memory compares use 16-bit signed words (cmp.w/bpl) as the ROM does. srx's prepare_matrix_program_slot uses int32 (differs only on wrap) [C ROM; srx difference noted, not fixed there].

- D29 Save repair fix: the ROM repairs slot 0's copies whatever slot is loaded (doc 07 §7). rules/save.lua repairs the loaded slot. Set ROM_REPAIR_QUIRK = true for byte-exact SRAM behaviour.
- D30 The IC type and rating (0x19B40 / 0x19BF6), alert changes (0x1989A), map reveal (0x533EE), slot cycling (0x19432) and logout timing are now Lua rules (rules/matrix.lua). The services current_ic_type/ic_rating are replaced by the RAM reads matrix_node/alert/in_alert/map_seen. Logout, map drawing and the screen restore stay C++ rendering, timed by the documented frame counts.

- D31 The Matrix combat and movement rules are now Lua (rules/matrix_combat.lua, matrix_move.lua): pool split, program rolls, Attack/Slow/Degrade, Smoke/Mirrors/Shield, IC turn kinds, passive/trace/attack IC, Blaster MPCP burn, crash check, link and go-to cursors. The C++ main loop 0x1ADE6 keeps the animation/timer flags (FFE0xx) and calls these at the ROM call sites. One deviation: Right on the 8th link wraps to 0 instead of reading +0x0F as a link (ROM quirk, documented).
- D32 Doc fix: the decker test is min(pool, rating) + rating. Doc 05 §8.4 said "+ pool". Music 05 is the node-menu theme.
- D33 Node actions 0x54AEA..0x5561E and the programs Rebound/Medic/Sleaze/Deception/Relocate/Analyze are now Lua (rules/node_actions.lua, matrix_programs.lua, datastore_special.lua auto-translated from 0x55058..0x5559C). The trace bar is in matrix_programs.trace_frame.
- D34 Fix: 0x1A154 crashes the IC (+0x0F = 0). Slow below reaction 0 now crashes it as well; this was [H: frozen] before.
- D35 Doc fix 05 §8.7: Transfer Data searches first (detect = (Int+Computer+Sensor)/3 vs rand(10)+1), then asks whether to download. The value is 0x16668A[rand(7)+rand(6)] x (colour-1); Red uses x5.
- D36 Analyze (0x56064): net = decker successes - roll(node security, max(Sensor,2)); FFFB30[FFF102] |= min(net,4)<<4. Dump 0x1B302 = 60-frame scroll shake (C++ presenter) then logout. Transfer run check = FFF0E3 < 0 and class == FFFBF2 (dropped my extra FFFBF2 >= 0 test); objective file value byte is the FF marker FFF0AE, not FFFBF6 (fix of my D33 code). MatrixGame service struct + node_action/run_program/matrix_frame entry points in menu_binding.hpp.
- D37 RNG: the ROM has ONE stream (FFF9F8: 0xD44/0xD3A/0xDBA). srx has a field seed from QRandomGenerator, plus separate title and jitter seeds; our debugger SeedSet assumed field+title. New srxdbg::SourceRng (single shared instance; ROM clamp of negative dice; per-call RNG events with site names) and SEEDS = {source}. Audit R1-R9 in docs/24.
- D38 Fix vs srx: the node-action gate 0x548BE uses min(pool, Computer) + Computer (srx matrix_node_action_loop_conversion.cpp:209 uses max). Lua node_actions.gate / random_alert (0x54A8E) were added. Matrix wiring sketch: host/matrix_game_srx.sketch.cpp.
- D39 RNG patches 0001-0006 for srx bfa3908 are in out/patches/ (unapplied). Cold-boot d1 = 0x0000FFFF (0x6DA), so the zero-RAM seed is 0xFFFF and the title seed is 0xD29F after the boot intro. Docs 24/25; coverage fixes (GEMS samples run to 0x52D65, code from 0x52D66).

- D40 StoryFlag enum (refactor/enums.lua E.StoryFlag), FFFC1E bits 0-31 with [C]/[H]. Commands 0x15-0x1D and cmd05 set "the current trigger's bit" (FFDD64), so the port must keep FFDD64 = active trigger index while a dialogue runs; otherwise bits 0-5 are never set. Doc 14 §3.2.
- D41 tools/fc1e_sites.py fixes: (a) FFDD64 imm writes inside a trigger handler override the loop index (bit 8 at 0x53F50, not bit 4); (b) a register reloaded from FFFC34 ends FFFC1E tracking. 0x1F8B4/0x1F8B8, cmd61 0x22664/0x22668 and cmd40 0x21F46 are FFFC34 bits, so bits 7, 18 and 19 have no setters there and bit 18 is never cleared; (c) command ids come from the ROM table 0x12CCB4 (bits 13 and 17 belong to cmd1C and cmd1B, bit 20 to cmd37). Audit [C static]: 0x1F246 and its direct callees never write FFDD64. Its only writers are the immediate stores just before 0x2196C and 0x1BA12 (Matrix scratch, move.w). Loop quirk [C]: 0x53A78 reloads the index from FFDD64 after each handler, so a handler that writes FFDD64 and returns d6>=0 moves the loop. Kept ROM-exact.

## Headless mode (mandatory for Luna)
Debugger-attached runs never show a window or take focus (docs/23 §9). The host checks `srxdbg::headless::parse/apply` first: `--headless` / `SRX_HEADLESS=1` sets `QT_QPA_PLATFORM=offscreen` before Qt starts, skips creating the window and opening audio, renders only into the VDP model or an offscreen image, lowers priority (nice 10 / BELOW_NORMAL), and writes PNGs only with `--png`/`SRXDBG_PNG=1`. The menu's Lua screens are tested with no Qt at all (`tests/menu_tests.lua`, `tests/submenu_tests.lua`). Host stub: `refactor/debugger/tests/headless_host_stub.cpp`.
