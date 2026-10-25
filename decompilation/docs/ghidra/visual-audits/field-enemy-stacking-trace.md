# Field enemy stacking: ROM emulator trace vs port (2026-10-25)

User report: enemies stand on top of the runner or each other. Port capture: runner 15584,11520; Weenrs stop at x=15455. Trace script/data kept outside the repo (/workspace/stack_trace).


## Trace setup and RAM layout

- [C] Ran the US Genesis ROM through `/workspace/shadowrun/emu/retro.py` from the existing `/workspace/shadowrun/emu/doors/hw2.state` Scene 2 savestate, with `SDL_AUDIODRIVER=dummy` and `SDL_VIDEODRIVER=offscreen`; no window or audio file/playback was used.
- [C] The 600-frame unattended trace is in `encounter_trace.tsv`, produced by `trace_encounter.py`. Each frame has runner slot 0 and both active enemy records; coordinates are sampled after each emulator frame.
- [C] The RAM actor array begins at `FF0100`, has 16 records at `0x100`-byte strides, and stores X/Y words at `+0x00/+0x04`; kind is word `+0x0E` and active flags are byte `+0x10` (Ghidra `actor-ai-heartbeat-source-audit.md`, `fn_00004618.cpp`). In this savestate the runner is `FF0100`; kind-8 Weenrs are slots 6/7 at `FF0700/FF0800`, both with flags `0x25`.
- [C] At trace frame 0 the runner was `(26834,19072)` (`0x68D2,0x4A80`), Weenr 6 `(26191,19066)` (`0x664F,0x4A7A`), and Weenr 7 `(26454,19074)` (`0x6756,0x4A82`). The runner stayed at its initial position throughout.

## Trace measurements

- [C] The ROM's relative-distance measure at `0x15D4E` is `max(|dx|,|dy|) + floor(min(|dx|,|dy|)/2)`; Euclidean values below are supplemental calculations from the same RAM coordinates.
- [C] Weenr 6's nearest runner distance was ROM metric **305** / Euclidean **286.4**, at frame 20 and thereafter: enemy-minus-runner delta `(-283,+44)`, position `(26551,19116)` (`0x67B7,0x4AAC`).
- [C] Weenr 7's nearest runner distance was ROM metric **343** / Euclidean **331.1** at frame 17, delta `(-330,+27)`. Its stable position from frame 20 was `(26479,19074)` (`0x676F,0x4A82`), delta `(-355,+2)`, ROM metric **356** / Euclidean **355.0**.
- [C] The closest enemy pair was at frame 19: Weenr 6 `(26526,19091)` and Weenr 7 `(26504,19099)`, delta `(22,-8)`, ROM metric **26** / Euclidean **23.4**. They never had exactly equal coordinates, but at that frame both were in the same `0x200`-unit map cell `(x>>9,y>>9) = (0x33,0x25)`.
- [C] Both enemies' X/Y positions stopped changing by frame 20 and remained fixed through frame 599 while the runner stayed put. [H] Their centers are close enough to suggest visible sprite overlap, but this coordinate trace does not measure sprite bounds or pixels.

## ROM stop and occupancy behavior

- [C] `0x13904` refreshes target/AI policy on the `+0xB6` timer cadence (21 field-service passes); `0xA8EA` calls the actor's `+0x3A` movement continuation on each service pass. The Ghidra C exports are `fn_00013904.cpp` and `fn_00013D0C.cpp`.
- [C] For ordinary kind-8 actors, `0x14C7C` calls `0x15C58`, which accepts a linked active, unlocked target only when the ROM distance metric is `<0x200`. The accepted branch at `0x141A0..0x141D8` copies the actor's current X/Y to `+0x96/+0x98`; when its action delay is negative it enters the `0xB1DC` target-action transition. Rejected targets enter pursuit at `0x141DE`, then `0x13D42`/`0x145A2` route toward the linked actor. See Ghidra C export `fn_00015C58.cpp` and disassembly at those addresses.
- [C] `0x1460E` is a separate route-arrival stop: `compute_diagonal_step_cost` below `0x1E0` clears route state and changes continuation to `0xB45C`. It checks distance to the current route destination, so it is distinct from the runner-engagement gate `<0x200` (`fn_0001460E.cpp`).
- [C] The movement collision probe at `0xB48C` calls only terrain collision `0xE7C8` and stores that result; the `0x145A2` obstruction path consults the `FF1700` terrain-tile map. These movement paths do not test another actor's coordinates for occupancy (`fn_0000B48C.cpp`, `fn_000145A2.cpp`). The trace's same-cell approach is consistent with that behavior.

## Port comparison

- [C] The port's `actor/movement.lua::resolve_source_target_motion` uses `distance < AiTuning.CLOSE_STRIKE_RANGE` (`0x200`) for ordinary creatures and sets `movement_active=false` when accepted (`movement.lua:318-378`; `enums.lua:271`). The port's target-distance helper uses the same max-plus-half-min metric as ROM `0x15D4E` (`movement_metric_conversion.cpp:58-78`).
- [C] Port movement probes call `game.movement.collision` (`movement.lua:532-545`); `FieldSceneRuntime` backs this with `evaluate_field_collision` and terrain/object collision data, not other live actor positions (`field_scene_runtime.cpp:3596-3607, 9442-9505`). This matches the ROM movement probe's lack of actor-to-actor occupancy avoidance.
- [C] No source/port difference was found in the ordinary Weenr approach acceptance threshold or actor-occupancy behavior, so no ROM-fidelity fix is indicated. The ROM trace itself permits near-coincident enemies.
- [C] The reported port X-only gap is `15584-15455 = 129` world units. [H] Whether the port's full ROM distance metric is below `<0x200` depends on the missing enemy Y coordinate; the trace also began at a different approach phase, so these data do not establish a stop-logic mismatch.

## Port stop measurements

- [C] Ran `/workspace/srx_int_build/shadowrun_ghidra_conversion_qt_qa --assets ../../../assets --field-random-encounter-spawn-qa /workspace/stack_trace/port_run` with `QT_QPA_PLATFORM=offscreen SRX_HEADLESS=1 SDL_AUDIODRIVER=dummy SRX_QA_ENCOUNTER_TIMELINE=1`; it completed with exit status 0. The existing [QA mode](<../../../ghidra_conversion/qt_runtime/tests/qa/ghidra_conversion_qt_qa_main.cpp:1363>) spawns Scene 2's ordered two-Weenr encounter and leaves the runner at `(15584,11520)`. The 15-frame position samples are in `port_run/timeline.tsv` (not committed), with image captures alongside it.
- [C] The first sampled final positions occur at frame 165 and repeat in all 230 samples through frame 3600: source actor 0 / Weenr 0 at `(15455,11595)` and source actor 1 / Weenr 1 at `(15455,11579)`. Their runner deltas are `(-129,+75)` and `(-129,+59)`.
- [C] At those positions the ROM metric distances to the runner are **166** for Weenr 0 and **158** for Weenr 1 (Euclidean **149.2** and **141.9**). Both are inside the `<0x200` gate. Their separation is **16** by the ROM metric and Euclidean distance.
- [C] The ROM trace's stable positions were **305** metric for Weenr 6 and **356** for Weenr 7, so the port positions are closer to the runner by **139** and **198** metric units respectively. The port pair spacing (16) is also below the ROM trace's closest pair spacing (26 metric / 23.4 Euclidean at frame 19; stable-frame-20 spacing was 93 metric / 83.4 Euclidean).
- [C] The searched field runtime has no `SRX_TRACE`/`SRX_LOG` actor-position logger; this QA diagnostic samples every 15 frames, so frame 165 is the first recorded final position, not an exact per-frame stop timestamp. No code patch was needed to obtain the requested coordinates.
- [H] The port's observed stop positions differ from the ROM trace by stopping substantially closer to the runner, while remaining inside the same `<0x200` acceptance range. This establishes a positional difference; the capture alone does not isolate whether route or service timing explains it, nor demonstrate a different acceptance threshold.

## Refresh-phase trace (port per-frame vs ROM per-frame)

- [C] Port, per frame (`SRX_QA_ENCOUNTER_TIMELINE=1 SRX_QA_ENCOUNTER_TIMELINE_STRIDE=1`, plus a temporary Lua print of the heartbeat state that was then removed): Weenr 0's ROM metric drops below `0x200` at frame 135 (496). At that point its `+0xB6` think timer (`behavior_timer`) is 17. Every pass from then until the timer runs out is a `wait` pass (`think.lua::apply_think_result` returns true), so it keeps walking. The next `combat` refresh comes 18 passes later, at frame 153 and metric 166. Only then does `resolve_source_target_motion` accept (`<0x200`), stop movement, and start the action. Weenr 1 follows the same pattern and accepts at 158.
- [C] ROM, per frame (`encounter_trace.tsv`, `+0xB6` column `behavior_delay`): Weenr 6 drops below `0x200` at frame 9 (metric 511), with `+0xB6` = 10. It keeps walking until `+0xB6` reaches 0 at frame 19. The refresh then copies its current X/Y into `+0x96/+0x98` (accepted branch `0x141A0..0x141D8`), and it stops at metric 305. Weenr 7 is already accepted when the trace starts (`+0x96/+0x98` = its own position).
- [C] Both implementations therefore test acceptance only on the `0x13904` 21-pass refresh, and overshoot the `0x200` gate by however many passes remain in the current refresh cycle. That is 10 passes in the ROM trace and 17-18 in the port run. The 139/198 gap comes from where the refresh cycle happens to be when the enemy crosses the gate. The spawn timing differs between the two runs: the ROM run uses an `hw2.state` savestate taken mid-approach, while the port uses a fresh Scene 2 spawn. Neither the stop condition nor the cadence differs.
- [C] The ROM's worst case is the same: crossing the gate just after a refresh gives about 20 passes of overshoot. At roughly 15-25 world units per pass, a ROM Weenr can end up about 300-500 metric inside the gate, or nearly on the runner. No Lua change is made.
- [H] Still open: per-pass step lengths differ slightly. The ROM trace shows 15 per frame, then 25. The port shows 16/16 diagonal, 20 cardinal and 15/15 diagonal. This was not traced further. A same-seed spawn (emulator vs port) would be needed to compare whole paths frame for frame.

## Conclusion

- [C] ROM has no actor-to-actor occupancy avoidance; enemies may overlap each other. No port change made.
- [C] The port stops inside the same `<0x200` gate, but 139/198 metric units closer to the runner than the ROM trace, with tighter pair spacing (16 vs 26 at the closest).
- [C] The gap is explained by refresh phase (see "Refresh-phase trace"). ROM and port share the same 21-pass acceptance cadence, so this is ROM behavior and no port change is made.
