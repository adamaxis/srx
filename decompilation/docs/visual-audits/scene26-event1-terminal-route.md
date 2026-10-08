# Scene-2 event 1 through the Scene-26 Security Terminal result

Date: 2026-10-07

**Status:** partial. One live `FieldSceneRuntime` now carries the ordinary
New Game route through scene-2 collision record 93, row 1 of the map event
table, scene-26 resource setup, the physical maglock/collision route, and the
D8 Security Terminal result. The QA seeds the passkey, Electronics skill, and
PRNG state. It does not prove how story gameplay supplies those inputs or the
exact Genesis appearance.

## ROM route and native state

Scene-2 record 93 at ROM `0x17314C` is the physical `0xC1` collision. Normal
movement publishes event 1; row 1 of the active scene-2 event table at
`0x172CDC` selects resource scene `0x1A` (scene 26). The source dispatcher at
`0x167AE` prepares the resource map and its floor selection. In this fresh
route the extracted resource bound is 3 and the selected floor index is 0.

The same runtime instance enters scene 26 at `(0x0A00, 0x0500)`. Ordinary
controller input traverses its event-29 elevator selector, both event-2
maglocks, and the collision-defined corridor to records 124/125 at
`0x1D9E46`. Their D8 value produces event `0x18`, two negative collision
probes, and the `0x20EC2` Security Terminal continuation. Passkey item
`0x14`/quality 2 is added to the live Joshua record as a test fixture so the
route can cross both locks.

Ordinary A selects Electronics after the source ten-frame prompt delay. The
test sets active Electronics skill `+0x8A` to 10 and the source PRNG seed to
`0x1234`; successful access displays the first-floor text selected from the
entry's preserved `FFE12C` value. It requests sound event `0x30`. The result
stays static through its 30-frame wait; ordinary A returns to scene 26 with
the floor state intact. The full return image equals the current field-only
render.

## Eight-question full-screen review

1. **Does it look correct?** I reviewed all 546 full-screen BMPs. The 500
   scene-26 route frames were inspected in ten chronological contact sheets;
   I also checked the terminal prompt, first-floor result, timed result, and
   field return at full resolution. The scene-26 route shows the expected map,
   actor, HUD, modal, and clean return. The scene-2 event-1 prefix still has
   the previously recorded actor/HUD overlap during its long eastward hold;
   that remains an unresolved visual anomaly.
2. **Does the look reflect the behavioral state?** The trace records the
   physical `0xC1` event, row 1, scene `0x1A`, resource bound 3, selected
   index 0, scene-26 spawn, both passkey door changes, D8 event `0x18`, the
   Electronics result, event `0x30`, and return with the same selected floor.
   Passkey, skill, and PRNG values are explicit QA fixtures.
3. **Are the visuals independently correct?** No matching Genesis capture
   shows this event-1-to-terminal route or its result. Independent section 03
   frames support the scene-2 spawn only. Internal assets and matching
   Debug/Release images do not establish Genesis pixel fidelity.
4. **Is it animated, and when?** The actor walks during held directional
   input. The terminal prompt holds for ten source frames before accepting A;
   the result is pixel-identical across its 30-frame source wait. No ambient
   animation was observed during the modal.
5. **Is there a sound, and how long?** Successful terminal access requests
   event `0x30`, which maps to persistent resource 048. This route verifies
   the request, not audible playback or its source stop lifetime; duration
   remains unknown.
6. **Are the other screen areas correct too?** The full images include the
   map, moving actor, HUD, dialog frame, text wells, and return. The modal
   replaces the field composition. The scene-2 actor/HUD overlap remains
   explicitly open.
7. **Do multiple frames support this interpretation?** Yes internally: the
   route captures all 500 scene-26 updates, the 15 scene-2 event-1 frames,
   the source prompt/result waits, and the return. All 557 Debug and Release
   test artifacts match by relative path and SHA-256. This remains one host
   route, not multiple independent Genesis examples.
8. **Are new controls drawn over stale controls?** The terminal result
   replaces the Electronics/Walk Away rows with a text-only page. After A,
   the full field return matches the field-only render, with no old dialog
   frame or choices left over.

## Verification and limits

`shadowrun_ghidra_conversion_qt_scene26_event1_terminal_qa` runs in Debug and
Release. Its output is
`build/conversion/qt/{debug,release}/qt_runtime/scene26-event1-terminal-qa/`;
`actual.bmp` is the first-floor result. The route keeps one native runtime
instance alive across both existing route paths and the terminal action.

This verifies the selected floor survives physical map traversal into the
generic terminal result. It does not implement a new game system. Story-driven
passkey acquisition, non-fixture Electronics outcomes, independent Genesis
visuals, the scene-2 actor/HUD overlap, and event-`0x30` playback/stop timing
remain open.
