# Shadowrun Genesis PC port — senior architecture review (Grok)

Baseline: Luna's `docs/architecture_audit.md` @ 8d9cadd8, read as a junior-dev pass.
Tags: **[C]** means I checked it in the tree at 8d9cadd8, or measured it with the command described. **[H]** means it is inference or estimation.
Paths are relative to `decompilation/` unless they start with `ghidra_conversion/`. All sizes are `wc -l` line counts unless they say bytes.

---

## 0. Executive summary

- [C] The ROM's game code is small. `export/functions.tsv` lists 911 functions totalling **111,098 bytes**, with the highest entry at 0x5DAFC, and `export/disassembly.s` is 29,141 lines. The port runs about **157k lines of C++** (qt_runtime/src 56,725 + src 66,603 + include 34,059) plus about 15k lines of hand-written Lua logic and 15k lines of generated Lua data.
- [C] **About 28.3k lines of C++ are unreachable from the shipped program.** I walked the include graph starting from every `qt_runtime/src` file and `src/scripting`. It never reaches **148 of 291 `src/**/*.cpp` files (17,280 lines) or 148 of 305 headers (11,032 lines)**. Those files carry **15,139 lines** of dedicated tests. Luna's §2.2 dead-code estimate was 2.9–3.4k, so the real figure is about **9× hers**.
- [H] Another **50 reachable src files (14,203 lines including headers)** export no function that qt_runtime/src or src/scripting ever calls. Their tests total 6,784 lines. Some of these may still be live through other conversion files, so each needs a link-level check before deletion.
- [C] The existing ROM coverage system is `ghidra_conversion/tools/rom_coverage.py` plus `docs/ghidra/rom-coverage-map.tsv`. The map has 1,080 rows: 1,064 marked "port" and 16 marked "replaced". Luna's §6 rebuilds a "seed" from comments and never uses this map. The map is citation-based, and its own docstring says it "does not establish that the runtime behavior matches the ROM". **796 of its rows point into `src/` conversion code**, so its apparent near-complete coverage is mostly coverage of the transitional layer, not of the code the game actually runs.
- [C] State ownership is split between C++ and Lua in at least four places: party/wipe, actors, menus and attack delay. The RNG has one state, but its draws come from 19 C++ files.
- Target shape [H, ROM-grounded]: a **host of 20–27k C++** and **23–41k lines of Lua logic**, plus the generated data that already exists. The arithmetic is in §3.

---

## 1. Verification of Luna's [C] claims

| # | Luna claim | Verdict | Evidence |
|---|---|---|---|
| 1 | §1 sizes: qt_runtime/src 56,725, src 66,603, include 34,059, qt field 42,283 | **Confirmed** | [C] `git ls-files … \| xargs wc -l` gives the same numbers. |
| 2 | One RNG state at 0x0D44/0x0D3A/0x0DBA with step `seed*0x20E5+1` | **Confirmed** | [C] `field_random_seed_` is a *reference* to `source_random_seed()` (`field_scene_runtime.hpp:2539`), so there is no second state. |
| 3 | RNG draws are mixed between C++ and Lua | **Confirmed, understated** | [C] There are 61 C++ draw sites across 19 files outside `random_conversion.cpp`. Examples: the `count_thresholded_random_rolls` users at `field_interaction_runtime.cpp:643/711`, `command_overview.cpp:2224` and `field_scene_runtime.cpp:10100`; `hotel_menu_runtime.cpp:178`; and the spawn heading at `field_scene_runtime.cpp:7029-7033`. [C] Lua re-implements the step in `enums.lua:498`, `effects/screen_effects.lua:97-104` (with its own `effect.random_state`) and `matrix_programs.lua:77`. |
| 4 | Attack delay 0xABFC is duplicated | **Confirmed** | [C] `actor/delay.lua` (128 lines) and `update_entity_animation_delay` are both live; the C++ side is called from `field_scene_runtime_player_actions.cpp:997` and `:1038`. `src/entities/entity_delay_conversion.cpp` (60 lines) duplicates the arithmetic. |
| 5 | `entity_action_resolution_conversion` and `entity_effect_spawn_conversion` are dead | **Confirmed** | [C] Neither is reached by the include graph. |
| 6 | The four purchase converters are dead | **Confirmed** | [C] All four are unreached. |
| 7 | `entity_action_math_conversion` has no Qt caller | **Confirmed, with nuance** | [C] Qt field files *include* the header, but none of its functions is called, so it is a type-only dependency. Its 1,370 lines can go once the shared types move. |
| 8 | `actor_behavior_decision` is dead | **Confirmed** | [C] Only its declaration and definition exist. |
| 9 | `World.actor_spawns_in_scene` has no caller | **Confirmed** | [C] |
| 10 | `field_mode_exit_conversion` is dead | **Confirmed, understated** | [C] Luna missed its siblings `mode_exit_conversion`, `mode_exit_resources`, `field_action_resource_selection`, `runner_action_result` and `matrix_program_purchase`, plus about 140 more files (§2.1). |
| 11 | Total removable is 14–22k | **Wrong (too low)** | [C] The unreachable code alone is 28.3k lines plus 15.1k lines of tests, and `archive/` adds 103,597 tracked lines (still present at 8d9cadd8). |
| 12 | §3: 6 emulator image comparisons | **Overstated** | [C] `visible_asset_pixels_match` is called 7 times and compares the port against **its own extracted assets**, which is circular. `image_matches_emulator_reference` has 3 hits including its definition. `capture_meter` has 2. Very few QA image checks actually compare against the emulator. |
| 13 | §3: QA main is about 69k lines | **Confirmed** | [C] 68,899 lines. `main()` starts at line 39,678, and there are about 220 `qa_case` invocations. |
| 14 | §5 step 1: route RNG draws "through named Lua calls in ROM order" | **Risky as sequenced** | [H] Draw order is decided by the control flow of the *consumer*. Moving the draw before moving its consumer adds a new ordering seam. See §4, step 1. |
| 15 | §6 coverage seed | **Redundant** | [C] The 1,080-row map and its CTest (`tests/rom_coverage_test.py`, wired in `ghidra_conversion/CMakeLists.txt:97-142`) already exist. Luna mentions the tool only once (audit line 296) and does not build on it. |

---

## 2. What Luna missed

### 2.1 Unreachable conversion code [C]
Method: an include-graph walk from the shipped Qt sources plus `src/scripting` (`/tmp/reach.py`; the method is reproducible from this description).

- **Unreached .cpp files by area:**

  | Area | Files | Lines |
  |---|---:|---:|
  | field | 32 | 3,210 |
  | entities | 25 | 2,839 |
  | world | 21 | 2,502 |
  | menus | 15 | 2,250 |
  | title_intro | 15 | 2,093 |
  | cyberspace | 14 | 1,516 |
  | core | 9 | 1,318 |
  | rendering | 7 | 606 |
  | session | 8 | 581 |
  | audio | 1 | 197 |
  | integration | 1 | 168 |
  | **Total** | **148** | **17,280** |

  The matching headers add 11,032 lines.
- I spot-checked 6 of these files. Each has 0 references in qt_runtime/src or scripting, and each has exactly 1 test.
- `src/core/remaining_routine_catalog_conversion.cpp` (643 lines, unreachable) is a hand copy of `export/functions.tsv` compiled into C++, so the same data is held twice.
- [H] The 50 symbol-level "never called" reachable files are led by:

  | File | Lines |
  |---|---:|
  | entity_action_math | 1,370 |
  | recruited_runner_service | 1,038 |
  | entity_animation_dispatch | 901 |
  | boot_sequence | 897 |
  | title_intro_transfer | 835 |
  | world_resource_batch | 610 |
  | entity_template | 510 |
  | word_transfer | 446 |
  | runner_roster | 432 |
  | title_intro_late | 403 |

### 2.2 The "conversion" layer is transitional and should not be an architecture tier [C/H]
- [C] `ghidra_conversion/src/**/*_conversion.cpp` is a per-ROM-routine translation, and 796 of the coverage-map rows cite it.
- [H] It is scaffolding: a place where routines landed before they had an owner. Its live parts belong in host modules (renderer, audio, assets, collision) or in Lua systems. The rest should be deleted along with its tests.
- [H] The `*_conversion` suffix and the `shadowrun_ghidra` namespace should disappear as files move.
- [C] The src/ vs qt_runtime/src split is not "engine vs UI". `qt_runtime/src/field` holds gameplay logic of its own (spawn rules, wipe countdown, legacy menus). So the split carries no ownership meaning today.

### 2.3 State ownership is split
- **Party/wipe:** [C] C++ owns `field_party_roster_` and `field_party_wipe_countdown_ = 0x3C // 0xABB6` (in `player_actions.cpp`, 3 references). `party_down.lua` (316 lines) owns the 0xB330/0xC64C/0xC716 wipe flow.
- **Actors:** [C] Lua `game.actors` has only 8 references, while C++ keeps 16-slot actor, render and target arrays. [H] The C++ arrays are therefore the real actor owner, and Lua works on a projected copy.
- **Menus:** [C] `menu/screens/legacy.lua` forwards to `host.legacy_open` and `host.legacy_advance` (`field_scene_runtime.cpp:9853-9858`). `field_scene_runtime_command_overview.cpp` (2,265 lines) is still a C++ menu.
- **RNG and delay:** see table rows 3–4 in §1.

### 2.4 The bridge is chatty [C/H]
- [C] The bridge has 28 `lua_pushcfunction`/closure registrations, 57 `lua_pcall` sites and 351 `game_scripts_.` calls from qt_runtime/src.
- [C] `game_script_runtime.cpp` is 16,662 lines and its header is 2,899 lines, with 127 structs and 82 public methods on `GameScriptRuntime`.
- [C] `ScriptActorBehaviorContext` (hpp:2339–2523) has about 91 fields. It is built and pushed for every actor heartbeat (12 `actor_heartbeat(` call sites).
- [H] That is precomputed C++ state pushed into Lua, which is the reverse of the agreed "Lua owns the logic" design. The target is a narrow host API (queries plus commands) that Lua pulls from as needed.

### 2.5 Header bloat and generated vs hand-written code [C]
- **Large headers:**
  - `qt_runtime/src/app/ghidra_conversion_qt_window.hpp`: 3,271 lines
  - `field_scene_runtime.hpp`: 2,986 lines
  - `game_script_runtime.hpp`: 2,899 lines
  - `frame_debugger.hpp`: 714 lines
- **Generated code:**
  - 12 Lua files, 14,807 lines
  - 5 files in qt_runtime/src, 1,298 lines
  - 1 file in src, 656 lines
  - 0 generated headers
- **Two enum generators feed C++:** `gameplay/scripts/tools/gen_enums_hpp.lua` and the `qt_runtime/src/srx_refactor/enums.hpp` header (977 lines, "GENERATED from gameplay enums and tools/native_boundary_enums.lua"). [H] These should be merged into one generator with one output.

### 2.6 QA code ships in the product [C]
- There are 407 references to `_for_dump`/`_for_qa` in shipped qt_runtime/src. One example is `open_gunderson_interaction_for_dump` (`services.cpp:996`), which is used only by QA main line 28119.
- `field_scene_runtime_debug.cpp` is 1,876 lines.

### 2.7 Build-target sprawl [C]
- There are 12, 50 and 2 explicit `add_executable`/`add_library` lines in the top-level CMakeLists, `qt_runtime/CMakeLists` and `conversion_modules.cmake` respectively.
- On top of those, **293 conversion test executables** are generated by a glob.
- [H] After §4 step 3, about 2/3 of those targets go away.

---

## 3. Target shape

Principle: the ROM's structure maps onto one Lua module per ROM system, plus a thin C++ host that does only what the Genesis hardware and BIOS-level routines did (VDP, Z80/YM, pads, SRAM) and the PC shell.

### 3.1 Measured ROM size by system [H: keyword bucketing of Ghidra names; bytes are C]

| ROM system | Functions | Bytes | Lua module(s) |
|---|---:|---:|---|
| actors / animation / AI | 199 | 24,044 | `actor/*` |
| field / world / scene | 129 | 20,884 | `field_*`, `world_*`, `building` |
| menus / shops / inventory | 127 | 16,710 | `menu/*` |
| matrix | 72 | 11,228 | `matrix_*` |
| boot / intro | 61 | 8,424 | `startup`, `title_intro` |
| dialogue | 23 | 2,480 | `dialogue*` |
| combat | 5 | 706 | `combat` |
| rng | 6 | 390 | **host** (one owner) |
| video | 83 | 9,586 | **host** |
| save | 14 | 1,214 | **host** I/O + Lua schema |
| audio | 21 | 688 | **host** (the Z80 driver is not in the export) |
| input | 11 | 484 | **host** |
| other / unclassified | 160 | 14,260 | split |

By address range [C]:

| Range | Functions | Bytes |
|---|---:|---:|
| 0x0xxxx | 262 | 29,324 |
| 0x1xxxx | 406 | 52,290 |
| 0x2xxxx | 20 | 1,992 |
| 0x5xxxx | 223 | 27,492 |

### 3.2 Lines per ROM byte, measured on the current tree [C]
For each family I counted the ROM functions whose entry addresses the code cites, then divided the family's line count by those functions' total bytes.

| Family | Lines | Functions cited | ROM bytes | Lines per byte |
|---|---:|---:|---:|---:|
| Lua `actor/` | 4,864 | 72 | 11,750 | **0.41** |
| Lua `menu/` | 7,831 | 169 | 34,480 | **0.23** |
| `combat.lua` | 1,346 | 22 | 4,078 | **0.33** |
| C++ qt field | 42,283 | 252 | 48,512 | **0.87** |
| C++ `src/` | 66,603 | 291 | 55,288 | **1.20** |

Lua expresses the same ROM logic in **2–5× fewer lines** than the C++ conversion layer.

### 3.3 Size estimates [H]
- **Lua logic.**
  - About 99 KB of non-host ROM code: 111,098 bytes minus the video, audio, input and save buckets (about 12 KB).
  - At 0.23–0.41 lines per byte, that gives **about 23k–41k lines**, against about 15k today (excluding data and `json.lua`).
  - **Inherent redundancy:** per-scene and per-run scripts that the ROM itself duplicates (johnson, building and hospital variants) should stay data-driven but will not shrink below what the ROM encodes.
- **Host C++: 20–27k lines**, built from these parts:

  | Part | Size | Basis |
  |---|---|---|
  | App, window and frame loop | about 3k | `app/` is 4,684 today; the 3,271-line window header gets split |
  | VDP compositor and painters | 6–8k | `rendering/` is 1,136, field paint is about 4.3k (`field_scene_runtime.cpp` 15255–19600), shop paint 973. Inherent: the hardware model. |
  | Audio (GEMS/YM command player) | 1.5–2k | qt `audio/` is 1,305 today |
  | Save/SRAM I/O | about 1.5k | persistence 782 + `field_scene_runtime_save.cpp` 1,075 |
  | Assets and TSV loading | about 3k | `world_content_tsv_loader.cpp` 2,762 minus its semantic decode |
  | Collision queries | 0.5–1k | |
  | RNG | about 0.1k | |
  | Lua bridge | 4–6k | today 16,662 + 2,899; shrinks to typed query/command tables |
  | Debug/QA hooks | out of the shipped target | test-only library |

- **Accidental redundancy (all removable):**
  - the conversion layer (§2.1–2.2)
  - the C++ copies of Lua logic (RNG consumers, delay, combat, inventory, purchases)
  - the hand-copied routine catalog
  - the two enum generators
  - `_for_dump` wrappers
  - the circular image checks
  - `archive/`

---

## 4. Migration plan

**Gate for every step:** `ctest` must pass, and QA must be **QA_GREEN** (the full QA main plus the GPGX reference fixtures). A step lands only if both are green; otherwise it is reverted.

Each step lists its estimated line change (Δ, all [H]) and how it is tested.

**Step 0: delete include-unreachable conversion code**
- Δ: −28.3k C++ and −15.1k tests (both measured [C]); about −148 CTest targets [H].
- Mechanical and behaviour-neutral by construction, because the shipped binary never includes these files.
- Test: the build plus QA_GREEN; ctest shrinks.
- Also delete the coverage-map rows that point at the deleted files, so the map stops claiming coverage it doesn't have.

**Step 1: single RNG owner (host)**
- Δ: about −300 to −600.
- Expose a single `rng.next()` / `rng.bound(n)` from the host, tagged with the ROM call-site address.
- Delete the step re-implementations in `enums.lua:498` (keep the constant only), `screen_effects.lua:97-104` and `matrix_programs.lua:77`.
- Leave the C++ consumers in place for now, but route all of them through the single owner.
- Test:
  - Add an RNG draw-trace fixture recording (seed, ROM site) per frame, compared against a GPGX trace for N scripted scenarios (combat roll, spawn heading, hotel, screen effect).
  - This catches ordering drift before any consumer moves.

**Step 2: single attack-delay owner (Lua `actor/delay.lua`)**
- Δ: about −150 to −250.
- Delete `update_entity_animation_delay` (called at `player_actions.cpp:997/1038`) and `entity_delay_conversion.cpp`.
- C++ asks Lua for the delay.
- Test: a delay fixture covering per-weapon and per-spell delays against GPGX frame counts (0xABFC/0xD7EA/0xD81C), plus QA_GREEN.

**Step 3: symbol-level dead check, then delete**
- Δ: up to −14.2k C++ and −6.8k tests [H].
- For each of the 50 files in §2.1, link with the file removed. Delete it if the build still links.
- Remove `remaining_routine_catalog_conversion.cpp`; `rom_coverage.py` reads `functions.tsv` directly.
- Test: link success plus QA_GREEN.

**Step 4: single party/wipe owner (Lua `party_down.lua`)**
- Δ: about −300 to −500 C++ and +100 to +200 Lua.
- Move `field_party_roster_` and the 0xABB6 countdown into Lua state, leaving C++ a read-only query.
- Test: party-wipe and revive fixtures (hurt-down animation timing, game-over transition) against GPGX.

**Step 5: legacy menu path**
- Δ: about −2.0k to −2.5k C++ and +0.6k to +1.0k Lua (using the measured menu ratio of 0.23 lines per byte).
- Port `command_overview.cpp` screens into `menu/screens/*` one at a time, then delete `legacy.lua` and `host.legacy_*`.
- Test: one menu fixture per screen (input script, then final menu state plus framebuffer hash against GPGX).

**Step 6: actor single owner**
- Δ: about −3k to −5k C++ and +1.5k to +2.5k Lua.
- Lua `game.actors` becomes authoritative; the C++ 16-slot arrays become render projections written once per frame.
- Replace the 91-field `ScriptActorBehaviorContext` push with host queries.
- Test: an actor-heartbeat trace (position, heading, animation, delay per frame) for 3 scenes against GPGX.

**Step 7: bridge narrowing**
- Δ: about −8k to −12k across `game_script_runtime.cpp/.hpp`.
- Replace the per-outcome structs with generic command and query tables, and generate the bindings from one schema.
- Test: unit tests for the bridge schema, plus QA_GREEN.

**Step 8: test restructure**
- **Δ:** −6k to −12k in QA main, on top of steps 0 and 3.
- **Fixture runners:**
  - Split the 68.9k-line QA main into data-driven fixtures (input script, expected trace or hash).
  - A single runner executes all of them.
- **Shared assertion library:**
  - Covers traces, framebuffer hashes, RAM slices and tolerance.
  - Replaces about 220 hand-written `qa_case` bodies.
- **Delete circular checks:**
  - Remove the 7 `visible_asset_pixels_match` checks, which compare the port with its own assets.
  - Wherever behaviour matters, replace them with emulator-reference comparisons (`image_matches_emulator_reference`).
- **Move `_for_dump`/`_for_qa` hooks out of the shipped target:**
  - The 407 hooks and `field_scene_runtime_debug.cpp` move into a test-only library.
  - This removes about 1.9–2.5k lines from the shipped code.

**Step 9: finish the ROM coverage map**
- Δ: about +200 to +400 tooling.
- Extend `rom_coverage.py`, rather than building a new seed:
  - (a) each row's owner must be a *reachable* file (reuse the include-graph walk as a CTest);
  - (b) add an `owner_kind` column: host, lua or data;
  - (c) add a `verified_by` column naming the fixture.
- A row counts as done only when it has a reachable owner and a fixture.
- Test: a CTest that fails on new unreachable owners.

**Step 10: delete `archive/`**
- Δ: −103,597 lines [C].
- Separate commit; the owner already agreed in pt101u.
- Test: the build plus QA_GREEN.

**Step 11: rename and collapse**
- Δ: about 0 net.
- Move live `src/` files into `host/{video,audio,assets,save,collision,rng,bridge}` and drop the `_conversion` suffix and the `shadowrun_ghidra` namespace.
- Merge the two enum generators.
- Test: QA_GREEN.

**Totals [H]:**
- **C++ removed:** about 28.3k measured (step 0) plus up to about 42k more (steps 3–8, including 14.2k from step 3).
- **Tests removed:** about 15.1k measured plus about 7–19k (step 3 tests and step 8).
- **Moved to Lua:** about 2.5k–4k new Lua lines replacing about 6–9k lines of C++.
- **Archive:** a further −103.6k lines.
- **End state:** a host of about 20–27k lines (versus 157k today).

---

## 5. Where I disagree with Luna

1. **Dead-code size.** It is 28.3k measured, not 2.9–3.4k. She checked a few named files instead of running a reachability analysis.
2. **Order.** Deleting unreachable code comes *first* (step 0). It carries zero behaviour risk and shrinks the surface the RNG and delay audits must cover. Her step 3 places it after the riskier RNG work.
3. **RNG approach.** Unify the *owner* and add a draw-order trace first, then move consumers along with their logic. Routing draws through Lua before the consumers move adds a new ordering seam.
4. **Coverage.** Build on the existing `rom_coverage.py` and its 1,080-row map, and make it reachability- and fixture-aware. Her comment-derived seed duplicates that work and inherits its flaw: citations are not behaviour.
5. **Where the test savings come from.** The biggest test win is deleting tests of dead code (15.1k measured), not shared-assertion refactors.
6. **Emulator checks.** Her count of 6 emulator comparisons is overstated. The 7 `visible_asset_pixels_match` checks are self-referential and must not count as ROM verification.
7. **Host size.** Her 25–35k host plus 35–40k Lua is not grounded in the ROM. The ROM is 111 KB of code, and the measured Lua density puts logic at 23–41k lines and the host at 20–27k [H].
8. **The src/qt_runtime split.** She treats it as engine vs UI. In reality both directories hold gameplay logic, so the split should collapse into host modules plus Lua.
