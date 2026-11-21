# STATUS: Company Man random-event response

Date: 2026-11-20
Branch: `port-fixes`

## Fixed

The `company_man_toothpick` random event now has a Lua response handler. ROM
entry `0x210B4` selects prompt `0x1214F4` and portrait/resource profile
`0x121E2C`. Attack, Insult, and Walk away resolve through typed gameplay
services, including the source Quickness/Charisma checks, alarm/Karma effects,
and the selected resource clue. The shared Insult success node `0x11CD92` is
treated as a terminal page because its first option is the `0xFFFFFFFF`
sentinel consumed by the no-options path at `0x1F592`.

## Changed files

- `decompilation/gameplay/scripts/field_random_events.lua`
- `decompilation/gameplay/scripts/dialogue_framework.lua`
- `decompilation/gameplay/dialogue/speakers.json`
- `decompilation/gameplay/tests_lua/field_random_events_tests.lua`
- `decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime_internal.hpp`
- `decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime.cpp`
- `decompilation/ghidra_conversion/qt_runtime/tests/qa/ghidra_conversion_qt_qa_main.cpp`
- `decompilation/ghidra_conversion/qt_runtime/tests/qa/ghidra_conversion_qt_qa_support.inc`
- `decompilation/ghidra_conversion/qt_runtime/CMakeLists.txt`
- `decompilation/docs/ghidra/field-random-event-selector.md`
- `decompilation/docs/ghidra/HILLS.md`
- `decompilation/docs/ghidra/decompiled-function-status.tsv`

## Validation

The old-tip Lua regression failed because the handler was missing. After the
fix, Debug and Release pass both `shadowrun_gameplay_field_random_events_qa`
and `shadowrun_ghidra_conversion_qt_field_company_man_toothpick_qa` headlessly.
The Qt QA follows the Scene-26 weighted selector into the prompt, sends B for
Insult, checks dialogue and rendered pixels, saves `opening.bmp` and
`actual.bmp`, and checks scene/camera/music restoration. Debug and Release
images match byte-for-byte. `git diff --check` passes. The full CTest suite was
not run.

Captures are internal Qt output, not independent Genesis evidence. The
Company Man transition cadence reuses the 83-frame random-event presentation
contract and remains a hypothesis [H]. Other random-event dialogue handlers
remain open. No push was made.

DONE


---

# STATUS: Cave exits and destination camera

Date: 2026-11-20
Branch: `port-fixes`

## Fixed

The cave mode-exit route continues through the typed transition when the ROM
gate is set, event is 8..11, no Lua script route claims it, and Lua resolves a
complete destination. `0x2CE0` publishes scene, room, and spawn while keeping
the current field mode.

QA now follows the source 64-by-64 collision map with ordinary direction input
from the production Scene-9 -> Scene-6 entry. It reaches Scene 45 / room 7,
Scene 7 / room 1, and Scene 45 / room 11 through room-0 south, room-7 south,
and room-1 east. The latter exact destination is `(0x5E00, 0x2280)`.

The route also exposed a camera regression: after destination scene prep
consumed the mode-transition spawn, the wrapper rewrote its old camera snapshot
over the newly centered camera. It now keeps the destination camera when the
spawn is consumed. Scene-45 playfields render in the internal capture.

Source evidence: `0x0A12`, `0x2CE0`, exit rows `0x14C0FE`, `0x1D4E5E`, and
`0x1B45A6`; Scene-45 profile pointer `0x3232[0x2D] -> 0x1D4BD0`, bounds words
+4/+5 `0x5300,0x2E00`; spawn recenter `0x030CC..0x030FE`.

## Changed files

- `decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime.cpp`
- `decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime_transitions.cpp`
- `decompilation/ghidra_conversion/qt_runtime/tests/qa/map_coverage_qa.hpp`
- `decompilation/ghidra_conversion/qt_runtime/CMakeLists.txt`
- `decompilation/docs/ghidra/HILLS.md`
- `decompilation/docs/ghidra/decompiled-function-status.tsv`
- `tools/map_coverage.py`
- `out/DIAG.md`
- `out/STATUS.md`

## Validation

Debug and Release pass the dedicated room-7 south, room-0 south, and room-1
east CTests (3/3 in each configuration). Each requires PASS, writes
`actual.bmp`, and rejects a fully black Scene-45 playfield using a 1,000
lit-pixel minimum. The three focused Debug/Release dumps match byte-for-byte.
The broader cave seed CTest also passes as a non-require-pass audit with 11
PASS and 2 MISSING rows in both configurations: the chained room-1 east case
stalls on the preceding room-0 south route at collision cell `(46,34)`, and
the room-2 south case stalls after room-5 south at `(26,21)`. Those chained
routes remain open. Captures are internal Qt output, not independent Genesis
visual evidence. No push was made.

DONE

## STATUS: Lone Star patrol Run choice

Date: 2026-11-21
Branch: `port-fixes`

## Fixed

Run now follows the shared ROM chase handler at `0x23782`: heat increases by
one before the roll and caps at eight; active runners' Quickness values are
averaged; target 4 increases by one for the lead runner's Lined Duster and by
two for Light Combat Armor. The 0-1, 2-3, and 4+ success bands return the exact
caught, partially caught, and escape text and select the ROM detective / Lone
12 actor lists as appropriate.

The field story-sequence publisher previously rejected screen-edge placement.
Run clears D0 at `0x237F8`, so `0x1E216` selects the existing `0x1E302`
screen-edge candidate path. The publisher now accepts that typed policy and
publishes the detective after the result dialogue returns.

## Changed files

- `decompilation/gameplay/scripts/enums.lua`
- `decompilation/gameplay/scripts/field_encounters.lua`
- `decompilation/gameplay/scripts/field_random_events.lua`
- `decompilation/gameplay/tests_lua/field_random_events_tests.lua`
- `decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime.cpp`
- `decompilation/ghidra_conversion/qt_runtime/tests/qa/ghidra_conversion_qt_qa_main.cpp`
- `decompilation/docs/ghidra/HILLS.md`
- `decompilation/docs/ghidra/field-random-event-selector.md`
- `decompilation/docs/ghidra/decompiled-function-status.tsv`
- `out/DIAG.md`

## Validation

Headless Debug and Release each pass `shadowrun_gameplay_script_runtime_qa`,
`shadowrun_gameplay_field_random_events_qa`, and
`shadowrun_ghidra_conversion_qt_field_lone_star_patrol_stop_qa` (3/3).
Physical QA drives the Scene-3 Run choice with controller input, checks the
caught result and heat 7 to 8, returns to the field, and verifies the detective
profile. The later Attack path also adds its three profiles with that detective
already present. Run result, returned field, and Attack field BMPs match
byte-for-byte between configurations. These are internal Qt captures, not an
independent Genesis comparison. The Lua matrix covers all three result bands;
physical QA for the partial-success and escape bands, nested caller, arrest
settlement, and combat completion remain open. No separate
`world-activity-terminal-outcomes.tsv` exists in this checkout; the current
outcome reference is `field-random-event-selector.md`.

DONE
