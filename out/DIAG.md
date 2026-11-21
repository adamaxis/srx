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

## Walkthrough 3 boundary: Salish travel rows are missing

The walkthrough 3 brief requires the actual cab/travel route from Redmond to
Salish before testing Owlfeather's `b2` trigger. The current ROM-to-runtime
coverage rows identify that route as `travel-taxi-r2-cursor0`: current cab-map
cursor 2, destination cursor 0, ROM selector inputs `0x0D4456+2` and
`0x0D445D+0`, then `0x020912 > 0x0136B6 > 0x0136DE`. Cursor 0 resolves to
Salish-Shidhe in `taxi_destinations.lua`.

That row and the six alternate destination rows for current cursor 2 are all
`MISSING`. The harness starts at `(0x74D7, 0x0A7F)` and reports no map
transition within 600 input frames after trying the south approach; the ROM
scene-entry position is unresolved in the row. The focused taxi QA passing
does not establish this real route. Walkthrough 3 therefore stops before
setting or asserting any of `FFFC1E` bits 2–4. No story state was injected.

## Edge-map mode expectation: street mode is preserved

The rebuilt full Release suite reported 14 edge rows as `PASS -> WRONG`, all
with correct event `0x10`, destination scene, and arrival position; the only
mismatch was expected mode 1 versus observed mode 0. The fixture's source gate
is already `FFDF40=0`.

**[C]** ROM bytes at `0x03168..0x031F0` compare `FFFBF9` with `0x10`, choose
the destination scene from `FFDDC4` and the camera split at `0x1400`, then
write the selected scene to `FFDDC4` and its scene-entry profile to `FFE114`.
The routine does not write `FFDF40`; with the documented street-mode input of
0, all 14 edge rows must retain mode 0. This is a test-data correction, not a
runtime transition change.

```text
0C 39 00 10 00 FF FB F9 66 00 00 7A
0C 79 00 00 00 FF DD C4 66 00 00 3A
0C 79 14 00 00 FF DD C6 65 00 00 1A
33 FC 00 05 00 FF DD C4 23 FC 00 00 32 12 00 FF E1 14
33 FC 00 03 00 FF DD C4 23 FC 00 00 32 22 00 FF E1 14
```

The active validation is `shadowrun_ghidra_conversion_qt_map_coverage_pass_field_qa`.

## Medicine Lodge Wise One scene profile was rolled back

The Release suite found that the quest-gated `0x1F` response showed its
direction text and started the normal return fade, but Joshua ended at the
pre-dialogue coordinates instead of the ROM profile position.

**[C]** ROM `0x21992` writes the longword `0x0001668A` to `FFE114`. The ROM
profile at `0x1668A` contains camera `(0x5000,0)`, Joshua `(0x5B00,0x0880)`,
and companions `(0x5B80,0x0900)` and `(0x5C00,0x0980)`. The following full
scene refresh consumes that one-shot profile. The port did apply it during
`0x2BD2` re-entry, then its generic modal snapshot restored the old party and
camera pose over it.

The modal restore now retains the newly applied pose when a pending scene
profile was consumed, while restoring the snapshot's activity, timing, and
audio state. Debug and Release passed the Medicine Lodge physical-entry,
Wise One physical-entry, and Hospital physical-entry QA cases. Their five
captures match byte-for-byte between configurations. `field-return.bmp` is
internal Qt output, not an independent Genesis reference.

## Cave exits use collision-map routes and preserve destination camera

Reproduced headlessly with `tools/map_coverage.py` on
`cave-room00-south-r14c0fe`, `cave-room07-south-r1d4e5e`, and
`cave-room01-east-r1b45a6`. Each route starts through the Scene-9 production
dialogue transition and is driven from the natural room spawn with ordinary
directional input. The QA plans a path over the imported 64-by-64 collision
map; it does not reposition the player to a trigger-side probe. The routes
reach Scene 45 / room 7 at `(0x0600, 0x2800)`, Scene 7 / room 1 at
`(0x0B00, 0x0780)`, then Scene 45 / room 11 at `(0x5E00, 0x2280)`, preserving
field mode 0 throughout. Each exit requires the exact scene, room, spawn, and
a fresh collision dispatch for its event.

**[C]** ROM bytes at `0x0A12` test `FFFC0E` and require `FFFBF9 >= 8` before
calling `0x2CE0`. The routine at `0x2CE0` consumes the event and `FFFC10` room
context and publishes the selected destination scene and pending spawn. ROM
rows `0x14C0FE` (room 0 south/event `0x0A`), `0x1D4E5E` (room 7 south/event
`0x0A`), and `0x1B45A6` (room 1 east/event `0x09`) resolve through Lua to the
checked destinations above.

The east route first failed because its event lies behind a curved corridor.
The new QA follows clear source collision cells with normal D-pad input and now
observes event `0x09` plus the exact room-11 destination. It also exposed a
rendering defect: the scene-transition wrapper copied its camera snapshot
before loading the destination, then wrote that old camera back over the
spawn-recentered camera. Scene 45 is profile row `0x2D` at ROM table `0x3232`,
pointer `0x1D4BD0`; profile camera bounds are `0x5300,0x2E00` (words +4/+5).
The source spawn routine at `0x030CC..0x030FE` recenters from the consumed
`FFE10A/FFE10C` position. The transition now synchronizes its typed camera
state when that mode-transition spawn is consumed.

**Validation:** Debug and Release pass the room-7 south, room-0 south, and
room-1 east CTests. Each writes `actual.bmp`; the Scene-45 playfield assertion
requires at least 1,000 lit pixels so a black destination fails. The captures
are internal Qt output, not independent Genesis visual evidence. Other cave
exits remain open. `git diff --check` passes. No push was made.

## Lone Star Run chase, ROM trace before implementation

Primary source is `decompilation/input/shadowrun_genesis_us.bin` (2 MiB, ROM
offset equals address). Raw bytes at `0x23750..0x23810` decode as follows;
`decompilation/export/disassembly.s` independently covers the shared
`0x0DBA` dice routine and the called actor-sequence placement dispatcher.

- `0x23750`: clear the Quickness sum and active count; iterate three `0x100`
  byte runner records from `0xFF0100`, test active bit 0 at `+0x10`, add the
  live Quickness byte at `+0x78`, then unsigned-divide the sum by active count.
  The raw sequence includes `08 2B 00 00 00 10` (BTST), `DC 2B 00 78`
  (ADD.W), `52 00` (ADDQ count), `D7 FC 00 00 01 00` (advance record),
  `51 CF FF E8` (DBF), and `8C C0` (DIVU). This is an integer average; class
  and race affect it only through the live Quickness in each runner record.
- `0x23782`: `52 39 00 FF FB FA` increments heat; `0C 39 00 09 00 FF FB FA`
  checks against 9; `6B 00 00 0A` takes the store path at 9 or above, writing
  8. `61 B2` then calls `0x23750`. The Run choice therefore raises heat by
  one, capped at eight, before calculating the chase.
- `0x2379E..0x237BE`: target starts at 4. The lead runner armor selector at
  `0xFF0176` adds one only for selector 5 and adds two only for selector 6;
  this is a lead-runner modifier, independent of support runners' armor.
  Typed item mapping uses `lined_duster` and `light_combat_armor` for those
  source selectors. The call at `0x237BE` passes average Quickness in D6 and
  the target in D4 to `0x0DBA`.
- `0x0DBA` rolls at least once, retries on a six while accumulating six, and
  counts each trial meeting its target. `0x237C4..0x237DC` branches at success
  counts 4 and 2: 0-1 selects one detective (`0x1D6FE4`, text record
  `0x120E80`, text pointer `0x11D3C1`); 2-3 selects Lone 12 then detective
  (`0x1D6FE0`, `0x120E88`, `0x11D3F8`); 4+ selects the escape result
  (`0x120E90`, `0x11D462`) without actors. Text extracted from the ROM is
  respectively “The detectives easily overtake you and begin firing.”,
  “You outrun some of them, but the others appear to be in better shape. They
  overtake you and open fire.”, and “You easily outrun the slothfull patrol.”
- The actor pointers at `0x1D6FE0` are `0x1D640A` then `0x1D649E`; the pointer
  at `0x1D6FE4` is `0x1D649E`. `0x237F8` saves the chosen text record, clears
  D0, and calls `0x1E1FC`; `0x1E216` sends zero D0 through `0x1E302`, the
  screen-edge candidate builder. If the actor sequence cannot start, the
  shared tail returns escape text `0x120E90`; the earlier heat increment is
  retained.

Caller review: the field patrol prompt at `0x20F12` and nested Company Man
patrol prompts at `0x20F5A` share command `0x99`. The undercover/police
dialogue routes at `0x20F8A` and `0x20FA2` also use this chase routine, so the
Lua resolver remains reusable by each already-supported caller. The Run
handler now implements the three success bands and named lead-armor modifiers.
The typed story-sequence publisher accepts the screen-edge policy selected by
the source D0 value. Debug and Release Lua plus physical field QA pass; the
physical route verifies the caught branch and returned detective actor. Other
result-band physical routes, nested-caller physical QA, combat completion,
and independent Genesis comparison remain open.
