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

## Eight-question full-screen review

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

Trace the navigable route from the actual scene-26 descriptor spawn through
the intervening collision barriers, identify the map transition that reaches
scene 26, and find independent terminal frames before making a Genesis visual
claim. The separate Security Terminal floor-result QA still injects event 2;
it does not replace this physical event-`0x18` test.
