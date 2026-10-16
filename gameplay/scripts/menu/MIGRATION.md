# Menu split: migration notes
The rules move to Lua (route.lua plus screens/*). C++ keeps only menu_binding.hpp: the presenter, input timing, sound output, display blank and window plane.
FSR = root `qt_runtime/src/field/field_scene_runtime.cpp`; bridge = `field_command_overview_bridge.cpp`.
Tests: `lua5.4 tests/menu_tests.lua` (27 checks).

## Active profile status (2026-10-15)
`field_init.lua` is the runtime registry. A script module counts as connected
only when this profile registers it and the live Qt route exercises its host
services. The Pocket Secretary root, Group Items, Dismiss Runner, Notebook,
Tips & Clues, Mr. Johnsons, Magic/Force/Posture routes, and the no-cyberdeck
response are Lua-owned. When the source deck-present flag is clear, row 2
pushes the generic `MESSAGE` screen with `NO_CYBERDECK`; Qt presents the
extracted three-line text and returns to the same selected row on a fresh
button edge. The message advances shared `E660` bezel animation, with QA
capturing and checking each of 30 full-screen wait frames. Application save
snapshots preserve the deck-present flag and typed deck record. The
Cyberdeck grid and program, info, and storage child routes are registered in
the active profile. Ordinary Pocket Secretary entry remains offline, as the
ROM sets FFF0B0 clear outside a Matrix session. Debug Pick System can start an
online Lua session and reach the Cyberdeck route, but the Matrix world
renderer, actor/IC heartbeat, and node-action handoff are not connected. The
System Map child now renders the ROM's 40x28 backdrop, six fixed labels, and
140 Lua-selected 2x2 cells using the map palette and tile tables. Its live
route test writes `matrix-map-route-qa/actual.bmp`, checks old-screen clearing,
green labels, and blank-cell transparency; backdrop and label placement were
reviewed against independent longplay map frames. The separate 4x4 Cyberdeck
grid's text and card art remain interim and have not passed an independent
visual comparison.
The Debug menu is registered in the live route and remains unlocked by the Qt
development override. Nuyen, Spells, Test Deck, Heal All, +10 Karma, Contacts,
Walk Fast, and Pick System have live route behavior. Pick System clamps to the
25 ROM system records, plays the jack-in sound, waits 20 frames, starts the
selected Lua Matrix session, and enters the online Cyberdeck route. Start
returns to a rebuilt Command page, and the debug row remains accessible. The
ROM system maps and node records are generated offline by
`tools/matrix_systems_extract.py`; runtime Lua consumes named records. This is
still a partial Matrix port: the active 3D world, actor/IC service heartbeat,
node-action loop, and Cyberdeck grid presentation are open. Test Deck writes
typed deck state and enables the deck-present branch. Save/Load is registered
in the live route and writes versioned JSON game-state snapshots through the
host persistence service.

The shared `0xC376` signed-byte row-wrap rule is in
`rules/cursor.lua` and serves the active Pocket Secretary family selectors,
including Notebook and its child lists. The presenter still owns the ROM
cursor-cell erase/draw operation.

## Moved blocks
| FSR / bridge location | what | new home | status |
|---|---|---|---|
| FSR 10498-10509 | per-frame wait/latch/sample | C++ stays: sample, then `route.frame(events)` | skeleton |
| FSR 10510-10571 | command-view Attributes/Skills child | `screens/attributes.lua` (`Attributes`, view-only); Qt presenter mirrors Lua cursor | **ported for command view** |
| FSR 10572-10576 | clips branch (advance 6049) | legacy CLIPS | port queue |
| FSR 10577-10581 | magic spell interface (advance 5914) | `screens/magic.lua` + `rules/magic.lua`; typed spell table host binding | **ported for menu, metrics, force/posture, and equipment selection**; field casting remains separate |
| FSR 10582-10598 | automatic_use 6581, first_aid 6597, inventory_trade 6400 | children of ITEM_ACTION (legacy) | port queue |
| FSR 10599-10602 | selector-suppressed latch | dropped: the route stack makes it unnecessary [H, verify after ITEM_ACTION port] | removed |
| FSR 10603-10612 | Cyberware full-screen wait/return | `screens/wait_any.lua` (`FULL`); Qt presenter builds the typed +0x90 rows | **ported** |
| FSR 10613-10625 | magical_defense_wait | `wait_any` (MAGICAL_DEFENSE); Magic pushes it on the class/resource fallback | **ported** |
| FSR 10626-10692 | posture dialog | `screens/posture.lua` | **ported** (fix D4) |
| FSR 10693-10938 | Pocket Secretary root and child loops | `screens/pocket_secretary.lua` owns root, Group Items, Dismiss Runner, Notebook, Tips & Clues, and Mr. Johnsons behavior; C++ projects typed source facts and draws the resulting state | those routes ported; other child loops remain queued |
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
- D15 Pocket Secretary root: only A (bit 14) dispatches; B does nothing (FSR treated B like A) [C 0xC2A6]. Lua owns root input/row, Group Items bit-list construction and fresh-edge wait, Dismiss Runner selection/action, Notebook/Tips/Mr. Johnsons input, filtering, waits, and return paths, plus the `0x18150` no-deck branch through generic `MESSAGE`. C++ supplies typed ROM facts and projects route state for painting. The deck-present Matrix branch remains an adapter boundary.
- D16 Magic: the force/posture loop is two stacked screens (MAGIC_FORCE, MAGIC_POSTURE) instead of a mode int.

- D17 Clips is ONE-SHOT per ROM 0x11B58: every outcome prints its text, waits for any edge (0xD94, silent) and returns to the Command page; Cancel / trade-Start print "Function Aborted!" (0xD1ED8, new MenuMessage.FUNCTION_ABORTED=21). B and C act like A; the main loop has NO Start check. Reload prompt: default No (0x11D00 clr + 0x127B0 eori), A only, "No" exits silently. srx/FSR kept the menu open and closed on B/Start: replaced [C].
- D18 Item action row 0x117D4: A or B confirm, C no-op, Start silent exit (was: B/Start exit) [C]. First aid 0x1235C: A/B/C confirm, Start silent exit [C].
- D19 Species attribute maxima now cite ROM 0xC22A6 (5x6, indexed +0x9A*6+i at 0xD8A4); a test compares the Lua table to rom.bin [C].
Sound sites for all of these: out/menu_sound_sites.txt (0x25 = move/toggle, 0x24 = confirm; Start paths silent) [C].

- D20 Automatic-use items (0x13, 0x14, 0x16, 0x18..0x2B: fetish, passkey, smart goggles, foci, talisman, spell lock, totems) only print "This item is automatically used." (0xD202F) and wait (0x12168 -> 0x121C0). The menu applies no effect, so nothing is missing from the port [C]. The passive effects are read elsewhere by code; that's outside the menu scope.
- D21 Pocket Secretary row 5 = the debug menu 0x599DA (docs/12). A temporary Qt development override exposes the normally title-code-gated, unlabeled sixth row and remains enabled for field testing. `screens/debug_menu.lua` owns its eight-row cursor, labels, and choices. Nuyen, Spells, Test Deck, Heal All, +10 Karma, Contacts, Walk Fast, and the Pick System route now use typed host services or Lua state. Spells writes the ROM's 14-entry spell row for caster archetypes and the Magic screen reads that row. Test Deck follows ROM 0x59B7E and seeds only the fields written by that routine. Selecting a wired action consumes the title unlock as ROM 0x59A64 does; the development override keeps the row available. The title result remains available as `StartupIntegrationState::title_debug_menu_unlocked`.

- D22 Magic rules and screens are registered in the active field route. Lua owns class/resource routing, spell selection, success/effect/defense values, force/posture input, equipment publication, and cursor cadence; C++ supplies all 14 typed ROM spell records and paints the resulting view. Field spell casting/target effects remain a separate gameplay route. Barrier's spell record, menu icon, equipped HUD art, and Lua text are now extracted from the ROM.
- D23 **partial:** Pick System retains the chosen system, waits the ROM 20 frames, and enters the Lua node-action route for 0x54662. The shared runner gate precedes Enter System links, CPU Go To Node, Cancel Alert, Lockout, Erase, Turn Off Node, and Crash System; Start enters the Cyberdeck loop, returns to the same node, and Pull Plug returns to Command. Every node-view rebuild now consumes the 0x54A8E rand(10) check against the accumulated action count; FFE16F seeds initial alert, passive/active notices use ROM text and sounds, and the 25/30-frame waits are handled by the route. Armed overload now performs the 10-frame 0x18776 fade and 54-frame 0x26A00 timing before its text, followed by 30 frames and input before exit. The host holds black during the transition; its source graphics remain open. Route QA writes full-screen node, alert, transition, message, Erase, and Crash System frames. Transfer Data, the live Matrix world/actor loop, and field-resume/death remain open.

## Timing contract [C unless marked]
- One `route.frame` per VBlank sample.
- Sounds and redraws happen in that same frame. ROM text writes happen during active display, right after the wait.
- A full build = `blank(OFF)`, then FULL, then `blank(ON)`. That gives at least one black frame; the exact count is [H] (it depends on the build length, not measured).

## Port queue (next)
Done in the active route: command/posture, item actions, Clips, Attributes/Skills, Pocket Secretary root, Group Items, Dismiss Runner, Notebook no-run flow, Tips & Clues, Mr. Johnsons, Magic/Force/Posture, Save/Load, and the connected Debug menu actions. The offline Cyberdeck grid and program/info/storage views are registered. Debug Pick System now clamps to the 25 imported ROM systems, preserves the selected system in Lua session state, waits the source 20 frames, and enters the online deck route. Its System Map child now uses ROM-derived tile art, cell words, palette selection, and labels; the interactive Matrix world is still unimplemented.
Open source/behavior boundaries: the ROM's `0x181F8`/`0x1822C` online entry and Matrix actor/IC loop, action dispatch and return-to-field path; Cyberdeck passcode entry; the drain-damage tail at `0x1046E`; `FFF0B1` inside the Matrix; `FFF101` save-state import; and child-loop migration.
In-game wiring: root `qt_runtime/src/field/field_scene_runtime.cpp` (`configure_field_menu_route()` supplies `ScriptMenuRouteBindings`). The field runtime now exposes typed Matrix run/field snapshots and accepts Lua commits for node-action effects. Live Matrix online/session/node facts and the actual active-run provider are still unavailable; `host/menu_game_fsr.sketch.cpp` remains an unbuilt design sketch.

Next: trace and connect the ROM Matrix session loop, then implement its world presentation and per-actor service cadence through the Lua state model. After that, connect the ordinary passcode entry and source return paths. The Notebook reads accepted-run availability from the Lua run registry (ROM `0xF2E2`/`0x1E72C`); its positive-current-run detail page and spell cast/target resolution remain open. Contacts and Shadowrunners list/detail screens are already registered and implemented.
Each one replaces its `legacy(...)` line in menu/init.lua.

- D24 Save/Load is registered in the active profile. Lua owns Save/Load and slot selection; the host stores two versioned JSON snapshots in the app data directory using atomic file replacement. Snapshots cover field location/player/camera, all three runner records and roster membership, funds, typed cyberdeck and spell-table state, story/map/notebook flags, script story facts, RNG/tick counters, and the Lua run registry including its active offer and per-run progress. Loading rebuilds the saved map resources before restoring state. This is application game-state persistence, not cartridge SRAM emulation. Dynamic field actor states and an active Matrix session are not yet included.
- D25 **partial:** the 4x4 Cyberdeck grid and mode input are registered in `field_init.lua`; debug jack-in passes selected Lua Matrix state to the node-action route, whose Start opens the deck loop. Ordinary Pocket Secretary entry remains offline. The host publishes the typed Cyberdeck record and five program slots. The active Matrix viewport and independent visual comparison remain pending.

- D26 **partial:** Program load/erase, deck info, and storage view/erase run through Lua and typed deck snapshots. Map/Retreat run from the debug Matrix session using selected-system Lua map and node records; a confirmed retreat returns to the node-action loop. The System Map paints the ROM backdrop, labels, and 14x10 tile layout from named assets; the Matrix world return path and full independent map-state comparison remain open.
- D27 The implemented no-slots, no-memory, not-on-run, and no-datafiles plain-rts text returns to the grid without a wait. Run-complete and no-retreat-node are still pending. Storage acts on A only and its erase choice defaults to Yes.
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
- D42 Matrix Cancel Alert (0x54AEA): Lua clears an active alert or preserves the no-alert state, then shows the matching ROM text stream at 0x1666A8/0x1666DD until any input edge (0xD94). The Qt presenter clears the prior node-action buttons before painting the message; route QA checks text row spacing and saves a separate frame.
- D43 The active node-action route now applies the ROM shared check at 0x548A0 before its wired Enter System, Go To Node, Cancel Alert, and Lockout dispatches: runner attributes/Computer, deck response, current mode, node colour/security, and one 0xDBA roll. Failure shows the exact 0x166AA5 lines and waits for any edge. Cyberdeck mode is mirrored into Lua session state while open and restored with the source saved mode on return. Crash System and the other action dispatches remain open.
- D44 Lockout (0x5561E) routes the existing Lua action result to ROM text 0x1669A2 and waits for any input edge. The decoded `0x80` separators preserve the three blank-row groups; the presenter supports the complete nine-row stream and clears the previous action buttons. The live route reaches ROM system 0's I/O Port node, verifies its Lockout row, then follows its first link back to CPU.
- D45 Crash System (0x54B34): ROM completion gate is signed FFF0E3<0 and FFFBF2<0; then FFFC28 bit 0 clears, FFF0E3=1, and FFFC1C bit 7 sets. Every path turns off the alarm, writes FFE170/FFE17F, draws 0x166A44 at $C518 and the countdown at $C722, waits 60 frames on 3/2/1/0, then calls fade 0x18776 and logout 0x26A00. Lua route runs the timed debug-menu sequence; the Qt presenter places the number between the text hyphens and clears the old actions. Debug/Release route tests save and review all four values. See `docs/ghidra/visual-audits/matrix-crash-system-countdown.md`.
- D46 Matrix world-state bridge and offer producer: `game.matrix_world_state()` returns field/world facts; the accepted type-6 objective is projected from and committed to `run_registry.active_offer.matrix_run_state`, which is included in the existing run save state. The field runtime no longer stores a parallel Matrix run record. ROM offers from initializers 6/7/8 now generate typed simple/moderate/expert objectives from the objective, target-system, and size tables. New Game uses context 0; the connected Wanderer interaction selects context 3. Focused rules QA checks all context rows and a deterministic Matrix objective. Visual offer captures, the other location-specific context routes (including scene 18/context 1), field completion/return settlement, alarm/security/maglock consumers, native fade, and live Matrix exit remain open.
- D47 Turn Off Node (0x54BD2): node class +1 dispatches 0x27 to alarm/security, 0x28 to security, 0x29 to maglocks, and 0x2A to alarm; the handler prints `NODE IS OFFLINE` at 0x16684A and waits at 0xD94. Alarm-off 0x1E09A conditionally changes actor +0x9E bit 1 to bit 2 and always stops resource 0x30 through 0x530D2. Lua reads `node.class` from the imported class byte and commits typed actor/field updates; the host has a distinct stop-resource callback. System 0 route QA reaches class 0x29; Lua rules QA covers each class and alarm actor flags. Field security/maglock consumers and unrecovered actor records remain open.
- D48 Erase (0x5559E): datastore Erase passes through the shared 0x548A0 gate. Lua checks active run state, node class against the run objective, and objective value; the objective path completes the run and prints 0x1668CE, while all other cases print 0x16685A. Both text streams wait at 0xD94 and return to the same node. Route QA covers both branches and clears prior action buttons; a missing active-run provider follows the ordinary erase branch without fabricating completion.
- D49 Transfer Data (0x54C5E) and special story files (0x55058): Lua runs the ordinary search, four 20-frame dots plus the 9-frame pause, typed Int/Computer/Sensor detection, all 70 ROM filename IDs, Yes/No choice, storage/five-slot checks, ordinary file writes, and matching run-objective branches. The prompt uses its own temporary cursor and restores the selected node action after its result message. The special handler receives FFFC34/38 from Notebook tips, FFFC1E world progress, FFFC22 group items, FFFC27 Mr. Johnsons flags, and optional wallet facts through the typed world-state bridge; unrestored tip/Johnson facts stay absent. System 18 uses only FFFC34. Systems 19 class 0 and 23 class 12 run only when FFFC27 bit 3 selects the known branch. Paths requiring FFFC26, FFFBFA/FB, or additional FFFC22 bits remain gated. System 16 class 1 requires wallet state and wraps its 50,000-nuyen award to the ROM's 32-bit balance. Debug/Release route and rules QA cover branch gating, the field fact projection, host-state round trips, and full-panel replacement. See `docs/ghidra/visual-audits/matrix-transfer-data.md`.
- D50 Matrix random alerts: 0x54A8E is connected to the node-action route and the shared source RNG; initial alert uses FFE16F, ordinary notices use ROM text/sound, and 25/30-frame waits are covered. Armed overload follows the ROM's five two-VBlank fade steps and 54-frame display-transition cadence before its message, with no extra alert SFX; Debug/Release QA checks the complete panel clear and wait/input order. The source transition graphics remain open. See [Matrix random-alert audit](../../../docs/ghidra/visual-audits/matrix-random-alerts.md).

## Headless mode (mandatory for Luna)
Debugger-attached runs never show a window or take focus (docs/23 §9). The host checks `srxdbg::headless::parse/apply` first: `--headless` / `SRX_HEADLESS=1` sets `QT_QPA_PLATFORM=offscreen` before Qt starts, skips creating the window and opening audio, renders only into the VDP model or an offscreen image, lowers priority (nice 10 / BELOW_NORMAL), and writes PNGs only with `--png`/`SRXDBG_PNG=1`. The menu's Lua screens are tested with no Qt at all (`tests/menu_tests.lua`, `tests/submenu_tests.lua`). Host stub: `refactor/debugger/tests/headless_host_stub.cpp`.
