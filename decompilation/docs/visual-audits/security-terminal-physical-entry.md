# Security Terminal physical-entry audit

Date: 2026-10-07

**Status:** partial. Ordinary field input now covers the scene-26 descriptor
spawn through the A2 electromagnetic-lock introduction. A separate checkpoint
QA covers the D8 Security Terminal modal and return. Lock actions, the D8 route
from the real descriptor spawn, and matching independent Genesis captures
remain open.

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

The route QA starts at scene 26's descriptor spawn `(0x0A00, 0x0500)` in
world mode 2. Four ordinary Right updates enter the paired event-`0x1D`
elevator trigger. Start returns to the current floor; six Down updates clear
the elevator lane, 13 Right updates pass its machinery, and 116 Down updates
follow the open corridor before Right crosses A2. This is scene-26 map
behavior, separate from the Stoker Motel interaction in scene 2.

The route reaches collision record 117 at ROM `0x1D9E00`. Its bytes are
`01 00 01 1F 01 60 01 7F 00 A2`, covering `x=0x100..0x11F`,
`y=0x160..0x17F`, with value `0xA2`. In world mode 2, the `0xB952..0xB994`
branch dispatches event 2 to the generic electromagnetic-lock continuation at
`0x20E48` when `FFE17F` is clear. The CTest asserts event 2, two negative
probes, the maglock continuation, activity selection 2, and the speaker-none
generic introduction containing “sturdy door” and the MORE option.

The lock action depends on active-party state. ROM `0x20E92` uses the event
selection to choose a lock grid or route values below 2 to the Security
Terminal continuation at `0x20EC2`; the native handler checks passkey,
Electronics, and Frag Grenade properties. This route does not choose an action
or verify a successful result. Collision records 3 and 5 place `0x7F` walls at
`x=0x100..0x11F`, above and below record 117's A2 lane at
`y=0x160..0x17F`; record 6 closes the lower map edge. The D8 records 124/125
are east of that barrier at `x=0x180..0x1BF`, `y=0x20..0x3F`. The ROM geometry
therefore places A2 in the passage toward D8. The D8 pair separately publishes
event `0x18` directly to `0x20EC2`; its current physical-input QA starts at a
source-coordinate checkpoint. Successful A2 action, crossing the passage,
and reaching D8 from the descriptor spawn still need ordinary-input QA. Debug
and Release CTest runs write 165 sequential 320x224 frames plus seven named
BMPs (172 total); all relative-path SHA-256 hashes match. The sequential
images were reviewed in 21 chronological sheets. These are internal
conversion images, not an independent Genesis reference.

### Eight-question review of the descriptor route

1. **Does it look correct?** The full sequence shows the scene-26 map, the
   elevator selector, return to the map, corridor movement, and generic lock
   page. The runner remains visible. Exact appearance is not established
   without a matching Genesis scene.
2. **Does the look reflect the behavioral state?** Yes for the captured host
   route: the trace records the event-`0x1D` elevator, return to scene 26, and
   event 2 at record 117. The CTest asserts the two negative probes and
   `maglock_door_interaction` continuation before accepting the prompt.
3. **Are the visuals correct?** The lock text and shared full-screen dialog
   are consistent with the source-selected generic prompt. The frame has no
   speaker portrait, matching an event/property prompt. No exact Genesis
   capture of this scene-26 lock page was found, so layout and palette fidelity
   remain open.
4. **Is it animated, and when?** The runner changes walk pose during movement
   and the map scrolls along the route. The lock page is identical across the
   ten-frame hold. ROM animation cadence has not been compared with
   independent gameplay frames.
5. **Does sound accompany it, and how long?** The Qt route emits no audio
   events. That records the host capture only, not ROM silence. Elevator and
   lock cues and their durations remain untraced.
6. **Are related screen areas correct too?** I reviewed the entire 320x224
   sequence, including runner, map, right HUD, selector, and dialog. No extra
   overlay or screen corruption was visible; Debug and Release outputs match
   by hash.
7. **Do multiple frames support the reading?** The 165-frame CTest sequence
   captures approach, elevator interaction, return, corridor route, trigger,
   and ten-frame prompt hold. This is one internally captured route, not
   multiple independent Genesis examples.
8. **Were old controls or graphics cleared?** The selector replaces the map
   while retaining the HUD, then Start returns to the map without the
   selector. The modal replaces map and HUD, with no stale field controls in
   the dialog. This is an internal redraw observation, not a Genesis
   comparison.

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

Trace and implement the active-party lock choices and results from this
descriptor route, then verify ordinary movement through the A2 passage to the
D8 event-`0x18` trigger on the far side. Identify independent Genesis frames
and trace ROM sound cues and durations before making visual or audio fidelity
claims. The existing D8 physical-entry QA starts from a source checkpoint.
