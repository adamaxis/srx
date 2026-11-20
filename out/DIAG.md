# DIAG — Company Man "Slot off, frag-face!!!" random event

## Reproduction on the current tip

The ROM's weighted event table can select `company_man_toothpick` in field
scenes 21–40. The host event dispatcher recognizes only four random-event Lua
tasks; `company_man_toothpick` is absent from
`field_scene_runtime_internal.hpp`. If this event is selected,
`FieldSceneRuntime::present_field_dialogue_event()` receives a null task name,
sets `failure_` to `field event has no Lua behavior: company_man_toothpick`,
and reports `field.random_encounter.dialogue_handler_unimplemented`. It
prevents the ROM prompt and its reply from reaching a handler. The focused
headless reproduction added to `field_random_events_tests.lua` failed on the
old tip as expected:

```text
ctest --test-dir decompilation/ghidra_conversion/build/conversion/qt/release
      -R shadowrun_gameplay_field_random_events_qa --output-on-failure
Company Man toothpick random event has a Lua response handler:
expected function, got nil
```

The quoted choice occurs in two events. The Phantasmal Watcher already has a
Lua handler and insult tests. The Company Man encounter is the unhandled
random event: its extracted event content and choice table are present, but it
has no Lua task or native dispatch entry.

## ROM trace

**[C]** At `0x210B4`, the ROM loads prompt record `0x1214F4` and resource /
portrait record `0x121E2C`; the entry bytes are:

```text
41 F9 00 12 14 F4  4B F9 00 12 1E 2C
13 FC 00 00 00 FF F1 0C  60 00 E1 7C
```

Prompt record `0x1214F4` points at the Company Man opening and the three
choices. Its commands are `0xE0` → `0x24846` (Attack), `0xE1` → `0x2489A`
(Insult), and a return to `0x121538` (Walk away). The extracted event table
lists this outcome in scenes 21–40; the insult label is the exact ROM text
`Slot off, frag-face!!!`.

**[C]** The Insult handler at `0x2489A` reads runner slot 0's `+0x7A` value,
sets target 7, and calls the ROM threshold-roll routine `0x0DBA`. Its entry
bytes are:

```text
42 86  1C 39 00 FF 01 7A  38 3C 00 07
4E B9 00 00 0D BA  0C 39 00 01 00 FF F0 DD
```

The ROM branches on the resulting success count: zero calls `0x1E04A` and
returns response record `0x121528`; exactly one returns `0x121530`; above one
stages text `0x11FFDB` plus one of six strings from pointer table `0x121540`,
indexed by `FFE12C`, then returns shared dialogue node `0x11CD92`. The six
table strings are “He says to check the first/sixth floor” variants; preserve
the selected resource index as a typed host fact, not as an asserted physical
floor.

**[C]** The other visible choice has its own source route. Attack at `0x24846`
calls `0x23750` for average active-runner `+0x78`, rolls against target 7, then
raises the alarm for results 0/1 or awards one Karma to each of the three
runner records for results above one. Walk away returns record `0x121538`,
whose text pointer is `0x12013E`.

## Planned native behavior

Route the event through the existing random-event dialogue task, keep the
three ROM choices and extracted opening text, and implement each choice in
Lua. Bind the event portrait from asset
`dialogue/portraits/renraku-offices-entry.bmp` (source resource `0x121E2C`).
Expose the current resource variant through a named field service for the
above-one Insult result. Use the existing alarm and party Karma host services.

## Fix and validation

Implemented the Company Man handler from ROM entry `0x210B4`. The old-tip Lua
regression failed before implementation because
`RandomEvents.present_company_man_toothpick` was absent. Lua now covers Attack,
Insult, and Walk away, including all threshold-result branches and all six
semantic resource labels. The native field task dispatcher connects the event;
its typed `field.resource_variant` service maps the selected resource choice
without claiming a physical floor. The speaker binds the existing
`renraku-offices-entry.bmp` portrait. The Dialogue Framework uses the existing
83-frame random-event transition contract; the Company Man cadence itself
remains a hypothesis [H].

The Qt QA enters resource scene 26 via activity selection, selects the
Company Man through the sum-1 cell, presses B for Insult, checks the third
resource clue and terminal response, and verifies restoration of scene,
camera, music requests, and the result boundary. It saves `opening.bmp` and
`actual.bmp`, and checks green text pixels plus colored portrait pixels.
Debug and Release captures are byte-identical:
- `opening.bmp`: `CE92262F872D64B54281DB6189EC32763AF43E23457A97A5FB9506BAB8D1E0FA`
- `actual.bmp`: `2C584A743374899CB2858D6C2906A875108D4969EAFC081F7E5A9E7A6CFE0640`

Headless validation passed in both configurations:
- `ctest --test-dir build/conversion/qt/release -R '^(shadowrun_gameplay_field_random_events_qa|shadowrun_ghidra_conversion_qt_field_company_man_toothpick_qa)$' --output-on-failure`
- `ctest --test-dir build/conversion/qt/debug -R '^(shadowrun_gameplay_field_random_events_qa|shadowrun_ghidra_conversion_qt_field_company_man_toothpick_qa)$' --output-on-failure`
- `git diff --check`

The captures are internal Qt output, not an independent Genesis comparison.
Other random-event handlers remain outside this fix.
