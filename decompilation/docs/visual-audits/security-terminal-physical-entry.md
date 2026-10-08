# Security Terminal physical-entry audit

Date: 2026-10-07

**Status:** partial. The scene-26 collision trigger and modal return are covered
by ordinary field input from a source-coordinate pre-trigger checkpoint. The
full route from the scene descriptor spawn remains unresolved, and no matching
independent Genesis capture was found.

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

## Descriptor-spawn route exploration

The separate exploratory capture starts at scene 26's map descriptor spawn.
Ordinary input first enters the paired event-`0x1D` elevator trigger. After
returning to the current floor, the route reaches collision record 117 at ROM
`0x1D9E00`. Its bytes are `01 00 01 1F 01 60 01 7F 00 A2`, covering
`x=0x100..0x11F`, `y=0x160..0x17F`, with value `0xA2`. In world mode 2, the
`0xB952..0xB994` branch dispatches event 2 to the generic electromagnetic-lock
continuation at `0x20E48` when `FFE17F` is clear. The final captured frame shows
the lock introduction page. This is scene-26 map behavior, separate from the
Stoker Motel interaction in scene 2.

The lock action still depends on active-party state. ROM `0x20E92` uses the
event selection to choose a lock grid or route a value below 2 to the Security
Terminal continuation at `0x20EC2`; the native handler checks passkey,
Electronics, and Frag Grenade properties. This exploration did not choose an
action, verify a successful result, or prove the field loop advances from the
lock result to the D8 terminal records 124/125. The 155-frame sequence is an
exploratory artifact, not a CTest or a completed route claim. All 155 Release
frames were reviewed as full 320x224 images in chronological sheets; the
matching Debug frames have identical SHA-256 hashes. These are internal
conversion images, not an independent Genesis reference.

### Eight-question review of the exploratory capture

1. **Does it look correct?** The route shows the scene-26 map, the elevator
   selector, the return to the map, the corridor, and the generic lock page.
   The runner approaches a vertical map edge in frames 120-154. Rechecking the
   full images shows it on the near side of that edge; I did not establish a
   clipping defect. Exact appearance remains unverified without a matching
   Genesis scene.
2. **Does the look reflect the behavioral state?** The route log shows scene
   26 at descriptor spawn, the event-`0x1D` elevator at frame 5, return to the
   current floor, and movement to `(3880, 6000)` by frame 154. The next image
   is the generic electromagnetic-lock introduction. ROM record 117's `0xA2`
   value and the mode-2 event path to `0x20E48` support that interpretation;
   the exploratory harness did not log an event-2 state row for the final
   prompt, so that last transition is source-correlated rather than a test
   assertion.
3. **Are the visuals correct?** The lock text and shared full-screen dialog
   are consistent with the source-selected generic prompt. The frame has no
   speaker portrait, matching an event/property prompt. No exact Genesis
   capture of this scene-26 lock page was found, so layout and palette fidelity
   remain open.
4. **Is it animated, and when?** The runner changes walk pose during held
   movement and the map scrolls along the route. The lock page is static in
   the captured frame. Exact ROM animation cadence for this route has not
   been compared to independent gameplay frames.
5. **Does sound accompany it, and how long?** The recorded route's
   `audio_events` values are zero. That records what this Qt capture emitted;
   it does not prove ROM silence. The elevator and lock sound cues and their
   durations remain untraced.
6. **Are related screen areas correct too?** I reviewed the entire 320x224
   sequence, including the runner, map, right HUD, elevator selector, and
   dialogue. No separate overlay or screen corruption was visible in this
   capture. The Debug images match Release by SHA-256.
7. **Do multiple frames support the reading?** Yes: all 155 sequential
   frames show the approach, event-`0x1D` selector, return, corridor movement,
   and lock-page transition. They support the internal route interpretation;
   they are not multiple independent Genesis examples.
8. **Were old controls or graphics cleared?** Frame 5 replaces the map with
   the elevator selector while retaining the HUD; frame 6 returns to the map
   without the selector. Frame 155 replaces the map and HUD with the modal
   dialog. No stale selector, map, or field controls remain in that dialog
   image. This is an internal redraw observation, not a Genesis comparison.

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

Exercise the event-`0x1D` elevator and record-117 A2 interaction from the
descriptor route with a real active-party state; trace the post-choice return
and determine whether a successful lock result dispatches the D8 terminal
records 124/125. Also identify independent terminal frames before making a
Genesis visual claim. The separate Security Terminal floor-result QA still
injects event 2; it does not replace this physical event-`0x18` test.
