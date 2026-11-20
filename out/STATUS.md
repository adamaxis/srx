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
