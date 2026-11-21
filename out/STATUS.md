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

# STATUS: Cave room 7 south transition

Date: 2026-11-20
Branch: `port-fixes`

## Fixed

The host exit orchestrator now allows a verified cave mode-table exit to
continue to the typed transition callback. The route is accepted only with the
ROM scene-mode gate set, an event code from 8 through 11, an unclaimed script
route, and a complete Lua-resolved destination. ROM `0x2CE0` then publishes
the destination room, scene, and spawn while retaining the current field mode.

The natural QA route now proves Scene 6 / room 0 -> Scene 45 / room 7 -> Scene
7 / room 1, with arrival `(0x0B00, 0x0780)` and carried mode 0. The QA no longer
uses the stale fixed arrival-mode column for cave exits.

## Changed files

- `decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime.cpp`
- `decompilation/ghidra_conversion/qt_runtime/tests/qa/map_coverage_qa.hpp`
- `decompilation/ghidra_conversion/qt_runtime/CMakeLists.txt`
- `tools/map_coverage.py`
- `out/DIAG.md`
- `out/STATUS.md`

## Validation

The dedicated `shadowrun_ghidra_conversion_qt_cave_room7_south_qa` CTest
requires PASS and writes `actual.bmp`. It passes in Debug and Release; the
result images have the same SHA-256. The broader cave seed CTest also passes in
both configurations. Headless tests used offscreen Qt and null audio. The
captured image is internal Qt output, not an independent Genesis reference.

`cave-room01-east-r1b45a6` remains MISSING because its final movement probe
does not produce a fresh collision event. That is separate from the room-7
transition fixed here. `git diff --check` passes. No push was made.

DONE
