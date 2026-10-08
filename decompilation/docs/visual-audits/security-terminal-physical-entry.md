# Security Terminal physical-entry audit

Date: 2026-10-07

**Status:** partial. Descriptor-spawn QA now follows ordinary field input
through the event-0x1D elevator, record-117 A2 lock, record-110 second maglock,
and the D8 event-0x18 Security Terminal prompt. The QA fixture seeds one
passkey in Joshua's live record. Other lock outcomes and matching independent
Genesis captures remain open.

## ROM trace and native state

Scene 26 collision records 124 and 125 begin at ROM `0x1D9E46`. Their bounds
are `(0x180..0x19F, 0x20..0x3F)` and `(0x1A0..0x1BF, 0x20..0x3F)`, both with
value `0xD8`. At `0xB7B8`, the blocked-movement path requires at least two
negative directional samples before it masks the raw collision to event
`0x18`; that event selects the typed `building_security_terminal`
continuation, corresponding to `0x20EC2`.

The QA begins at world position `(0x1A00, 0x0500)`, immediately south of those
records, with the camera aligned to the checkpoint. It presses north through
the normal field controller. `route.tsv` records event `0x18`, two negative
probes, continuation `building_security_terminal`, and result
`building_reaction_started` before the handler clears the transient collision
state and opens the shared Security Terminal panel. The checkpoint is explicit:
it does not demonstrate how normal map navigation reaches scene 26.

Release artifacts are under
`build/conversion/qt/release/qt_runtime/field-security-terminal-physical-entry-qa/`;
Debug artifacts use the matching `build/conversion/qt/debug/` path. Both runs
produce 23 BMPs with matching SHA-256 hashes.

## Descriptor-spawn route QA

The route starts at scene 26's descriptor spawn `(0x0A00, 0x0500)` in
world mode 2. Four ordinary Right updates enter the paired event-`0x1D`
elevator trigger. Start returns to the current floor; six Down updates clear
the elevator lane, 13 Right updates pass its machinery, and 116 Down updates
follow the open corridor before Right reaches collision record 117's A2 lock.
This is scene-26 map behavior, separate from the Stoker Motel interaction in
scene 2.

Record 117 at ROM `0x1D9E00` has bounds `x=0x100..0x11F`,
`y=0x160..0x17F`, and value `0xA2`. The `0xB952..0xB994` route requires two
negative movement probes, publishes event 2, and selects the generic
`0x20E48` lock prompt while `FFE17F` is clear. The prompt is speaker-none,
contains the sturdy-door text and `< MORE >`, and remains pixel-identical
through its ten-frame hold.

For this route only, the QA gives the active Joshua record a Maglock Passkey
(item `0x14`) with quality 2 after asserting no active party member already
has one. Movement and buttons still go through the ordinary field controller.
ROM `0x21628` scans the three live runner records, checks item `+0x66` and
quality `+0x6E`, then accepts quality greater than or equal to the current
selection. Success calls `0x21616`, requests sound `0x4B`, and sets
`FFE16E=0xFF`. The result text says the passkey decodes the lock and disables
the door. The test holds this option-free information page for the source
30-frame wait, acknowledges with A, and checks a clean return to the field.

The field return then commits the negative result. ROM `0x2C1A..0x2C34`
uses `FFE11A` to increment the selected `FF1700` event counter as
`(old & 0xE0) + 1`. `0xB994` copies the most recently sampled `FFDFE4` terrain
index into `FFE11A`. The native field collision sampler now publishes that
index into the typed world-activity state, and the interaction return resumes
`0x2BD2`. In this route, sampled map index 712 changes from `0xA2` to `0xA1`.
That is the state transition which allows ordinary Right input to carry the
runner from `(0x0F50, 0x1770)` through the passage to `(0x1220, 0x1770)` in 18
updates. Crossing clears that event marker from the active collision buffer;
the later route checks the actual traversable map rather than expecting the
old buffer slot to remain `0xA1`.

The route then reaches record 110 at ROM `0x1D9DBA`, a second value-`0x82`
maglock with bounds `x=0x1A0..0x1BF`, `y=0x100..0x11F`, and event 2. The same
live passkey opens sampled map index 525 (`0x82 -> 0x81`) and requests `0x4B`
again. From that door, ordinary input follows the 64x64 collision map around
the `0x7E` wall: row 6 to column 16, north through row 2, then west into the
D8 trigger lane. Records 124/125 at ROM `0x1D9E46` publish event `0x18` after
two negative probes and select `building_security_terminal`. The route opens
the source Electronics/Walk Away prompt from the descriptor spawn; it no
longer relies on the checkpoint-only QA for reachability.

Debug and Release each write 500 sequential 320x224 frames, 22 named BMPs,
and seven route, collision-map, source-boundary, and audio files. All 529
CTest artifacts match by relative-path SHA-256. Ten chronological contact
sheets and full-resolution prompt, result, return, and terminal screens were
inspected. `actual.bmp` is the held D8 Security Terminal prompt.

### Eight-question review of the descriptor route

1. **Does it look correct?** The reviewed frames show the scene-26 map,
   elevator selector, both lock prompts/results, the passage and corridor, and
   the D8 terminal prompt. This confirms a clean internal composition; it does
   not establish the exact Genesis appearance.
2. **Does the look reflect the behavioral state?** Yes for this host route. The
   trace records both event-2 locks, two negative probes at each, map cells 712
   and 525 changing from `0xA2` to `0xA1` and `0x82` to `0x81`, then event
   `0x18` with two negative probes at the terminal.
3. **Are the visuals correct?** The lock prompt and passkey result are generic
speaker-none full-screen pages with the source text and no choices on the
result. Debug and Release match. No independent Genesis image of this scene-26
lock interaction was identified, so palette and layout fidelity remain open.
4. **Is it animated, and when?** The runner walks only during held directional
input; the source walk selector advances on the traced six/seven-update
cadence. The introduction holds static for ten frames and the result for its
30-frame wait. No independent capture is available to check exact Genesis
cadence here.
5. **Does sound accompany it, and how long?** Passkey success requests event
   `0x4B` once per successful passkey use (two requests total). It resolves to
   extracted resource 075, whose FLAC contains 97,770 samples at 44,100 Hz
   (2.217 seconds). The route verifies the requests, not speaker loopback or
   the hardware playback tail. The terminal prompt's sound cue and stop
   lifetime remain untraced.
6. **Are related screen areas also correct?** The review includes the entire
   320x224 image across the elevator, both locks, passage, corridor, and
   terminal. The runner remains visible in these host captures; no screen-wide
   corruption or stale controls were found. This says nothing about the
   separate scene-2 event-1 HUD overlap.
7. **Do multiple frames support the interpretation?** Yes for the internal
   route: all 500 sequential frames were reviewed, including both passages,
   each ten-frame introduction hold, both 30-frame result waits, and the
   terminal entry. This is one route, not multiple independent Genesis
   examples.
8. **Are new controls drawn over stale graphics?** No in these captures. The
elevator selector replaces the map and Start restores the field. Both lock
pages replace the map and HUD. After acknowledgement, the field render equals
the field-only render, so no old modal controls remain over the map.

## Eight-question review of the checkpoint captures

1. **Does it look correct?** The complete 320x224 captures show the map and
   runner before contact, the two-choice Security Terminal message, and the
   field after Walk Away. This is an internal conversion review; there is no
   independent Genesis frame for this terminal to establish visual fidelity.
2. **Does the look reflect the behavioral state?** Yes. The physical `0xD8`
   rectangles produce event `0x18`; the typed result records two negative
   probes and the Security Terminal continuation. The panel has the source
   Electronics and Walk Away choices.
3. **Are the visuals correct?** The full-screen modal replaces the field
   graphics while open. Walk Away restores the complete pre-prompt field
   frame, including runner and HUD. The host assets are source extracted, but
   their exact Genesis appearance remains unverified here.
4. **Is it animated?** The runner uses the existing field walk poses during
   approach. The prompt remains static over its ten-frame input delay. A
   matching independent capture for the terminal's source cadence was not
   found.
5. **Is there a sound?** This route requests no Qt audio events. A source sound
   cue and its stop lifetime have not been traced, so audio fidelity remains
   open.
6. **Are the other screen aspects correct?** The review includes the map,
   moving runner, portrait/sidebar, modal, and return. The modal owns the whole
   screen; no field controls or map pixels remain over its text and choices.
7. **Do multiple frames support the interpretation?** Yes. Sixteen sequential
   BMPs capture the approach, modal hold, and return; seven named full-screen
   BMPs preserve the checkpoint, contact, underlay, prompt, hold, and return.
   Debug and Release captures match. No independent recording shows this
   terminal interaction.
8. **Are new controls drawn over stale controls?** No. Opening the prompt
   replaces the field view. The return is pixel-identical to the saved field
   baseline, and the QA requires the runner to be visible again afterward.

## Remaining work

Exercise passkey failure, insufficient quality, Electronics, and other
active-party lock outcomes as source evidence permits. Find independent Genesis
frames for scene 26 and trace terminal sound playback/stop behavior before
making stronger visual or audio-fidelity claims. This route begins at scene 26;
it does not verify the preceding scene-2 resource-scene entry or a full
story-driven path from New Game.
