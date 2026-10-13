# Random encounter and creature targeting review

## ROM evidence

- `0x59986` selects a field outcome from two source bounded draws; Lua's
  `field_encounters.trigger_attempt` now applies `0x1E166`'s blocker,
  cadence, and probability gates before the host consumes the source PRNG.
- A zero-cell result reaches `0x1E216`; `0x1E25A` initializes each listed
  actor, sets the encounter flags, writes its edge coordinate, and schedules
  the shared timeout. `0x1E302` uses live camera words `FFDDC6/FFDDC8` and map
  bounds `FFDDCE/FFDDD0`; `0x1E41E` checks the normalized terrain coordinate.
- `0xA8EA` services actor behavior separately from `0x4618`'s render pass.
  `0x13904` refreshes actor target policy on its source cadence, while the
  continuation advances accepted movement on each actor-service pass.
- `0x14492` chooses the target pool from actor `+0x10.3`; it does not select
  the nearest screen sprite. B's newly pressed bit reaches `0x1DC4C`, which
  scans the source record pool and sets the selected actor's `+0x11.4` latch.
  Lua now owns that normal/special pool scan from typed record facts; the host
  applies the returned actor ID to the selected record. Lua also initializes
  `+0xA6` for the `0x5C7E` marker animation. The B path itself makes no sound
  call. The separate automatic target-service call from `0x1DD94` remains on
  the typed native conversion path.

## Runtime evidence

Debug and Release CTest
`shadowrun_gameplay_script_runtime_qa`,
`shadowrun_ghidra_movement_metric`, and
`shadowrun_ghidra_conversion_qt_field_random_encounter_spawn_qa` both passed.
Each field QA run injects selector rolls `1+2` for Scene 2's sum-3 row and
checks the two-Weenr sequence, active actor records, source entry cue requests,
Lua target link, close-range movement, creature walk frames, and B selection
surviving button release. The script-runtime CTest checks the four action-code
marker groups, the 64-to-0 shrink sequence, the persistent marker, and latch
clearing.
It now also checks the Lua-owned B scan's normal cursor advance, reserved-record
skip, normal restart at slot 3, and special-mode active-only scan and wrap.
The field QA matches all 14 opaque source pixels in each of the four 8x8
marker corners and confirms the right-side HUD is unchanged. These are not
tests of the production random-roll sequence or the full combat route. The
script-runtime QA in both configurations separately checks each
`0x1E166` blocker, the `(frame_counter & 0x3FF) == 10` cadence, threshold
accept/reject behavior, the sequence-forward branch, and that Lua requests
the source probability draw only after state/cadence eligibility passes. The
movement-metric test compares all eight heading sectors against the ROM basis
table.

The real edge-spawn captures (`encounter-entry`, `after-30`, `after-180`, and
`after-480`) keep the full map and HUD composition and contain no visible
creature because the source candidates are beyond the current viewport. The
ROM heading correction found during this review changed the natural trace:
both Scene 2 actors now turn through the runner's area instead of continuing
away. They remain outside the viewport after 480 service frames, so their full
approach and arrival remain unresolved. This conversion evidence uses the
injected `1+2` selector result, not an independent encounter recording. The
separate `encounter-visible-qa-fixture.bmp` moves the runner close to the
first creature to inspect its approach and walking art; it is a QA fixture,
not a simulated gameplay route. `creature-walk-frame-0.bmp` and
`creature-walk-frame-3.bmp` show distinct source walk poses. The target-marker
capture keeps the production spawn frames untouched, then repositions the
selected live record beside the runner only for an on-screen renderer check.
`target-selection-b.bmp` captures the shrinking stage and
`target-selection-b-static.bmp` captures the settled selector. Both retain the
full map and HUD; the HUD crop remains identical to the baseline.

ROM `0x5C7E` selects shared object-atlas tiles `0x5E5..0x5E7`, the palette line,
and four flipped 8x8 corner descriptors. The importer now exports each scene's
four variants as alpha-preserving PNGs. Lua owns the `+0xA6` render-pass
countdown: the first frame begins at radius 64, then the corners move inward by
8 pixels per render pass until the four-corner marker remains at radius zero.
The renderer draws those corners after the actor body to preserve the Genesis
SAT's earlier-sprite overlap priority. B itself has no sound call.

Independent combat footage in section 06 corroborates the marker shape:
`frame_002190.png` and `frame_002220.png` (9:43-9:44) show yellow corner
brackets; `frame_002430.png` and `frame_002460.png` (9:51-9:52) show red
brackets around a combatant. This confirms the separated-corner treatment and
that the hue varies in play. The footage does not expose actor action bytes, so
it cannot independently assign those hues to a specific action-code group or
prove a frame-for-frame match with this Scene 2 fixture.

## Eight-question screen review

1. **Does it look correct?** The extracted walk poses differ as expected, and
   the settled marker is a slim four-corner selector rather than a filled box.
   Independent combat frames show the same separated brackets in yellow and
   red. The exact Scene 2 hue is source-palette-derived, not matched to that
   different combat encounter.
2. **Does the look reflect the behavioral state?** Lua applies the B-edge
   source-pool selection rules and returns the selected actor ID. The host
   publishes the matching `+0x11.4` latch, and Lua retains it after release,
   advances `+0xA6`, and returns marker style from the actor action byte. The
   pixel QA checks each of the four drawn corners, not just the state flag.
3. **Are the visuals correct?** The marker tiles, palette lines, flips, and
   descriptor offsets are from the ROM. PNG alpha prevents black tile
   backgrounds. The independent frames corroborate shape, but not the exact
   Scene 2 palette/action pairing.
4. **What animates, and when?** Creature movement uses named C062 directional
   frames on the render heartbeat. Its route is selected every 21 actor-service
   passes; the current heading turns toward that route by `0x0800` on each
   `+0x3A` continuation pass. The marker shrinks 64, 56, ... 0 on render
   passes and stays static while the target latch remains set; clearing the
   latch removes it.
5. **What sound accompanies it?** B selection makes no ROM sound request. The
   tested entry sequence requests `0x2E`, which resolves to a verified silent
   or no-op resource; no audible duration is claimed.
6. **Were neighboring areas checked?** All 12 full 320x224 QA frames were
   opened individually, including production edge-spawn frames through 1,200
   actor-service passes, the close fixture, two walk poses, and both selector
   stages. The full HUD remains
   unchanged. Debug counterparts match the reviewed Release frames by SHA-256.
   Both natural-spawn creatures remain outside the viewport in the long trace,
   so this capture does not verify their approach or attack behavior.
   The shrinking frame intentionally has widely separated corners; the settled
   frame has the compact selector around the target.
7. **Are there multiple frames?** Yes: 0, 30, 180, 480, and 1,200 service frames,
   two walk poses, selector shrink and settled frames, plus independent
   section-06 longplay frames 2190, 2220, 2430, and 2460.
8. **Are new UI elements covering stale ones?** The selector adds only the
   four source corner sprites to the field layer. Their transparent pixels
   remain transparent, the marker follows the selected actor, and the existing
   HUD is pixel-identical; no old menu or button layer is reused here.

## Creature target acceptance review

1. **Does the actor stop when the ROM accepts a linked target?** Yes. At the
   source range gate the actor leaves pursuit mode, records its target heading,
   and does not emit a movement step. The Qt handoff accepts this only when Lua
   returns `ai_target_engaged`; otherwise a requested movement heading still
   requires its collision-resolved step.
2. **Does the state match the kind-specific source branch?** Ordinary actors
   accept below `0x200`; Hell Hounds accept below `0x300` through their
   `+0xDC`/effect-slot gate and from `0x300` through `0x5FF`; Gargoyles and
   Wendigos accept below `0x200`. Gargoyle frame 2 sets its source animation
   accumulator and keeps routing for that pass. Boundary checks run in both
   build configurations.
3. **Are the direction and action visuals fully verified?** The test checks
   the Lua target heading and current-heading fields. The current field
   capture can show a creature and its target marker, but there is no
   independently identified Genesis sequence for this exact creature action.
   Attack animation, hit resolution, damage, and death remain open.
4. **Are other screen regions and overlays clean?** All 12 random-encounter
   frames and all 8 Hound-effect frames were reopened at 320x224. Debug and
   Release hashes match for each named frame. The entire right HUD remains
   intact; no stale menu or button layer appears. At the shrinking-marker
   frame, lower corners fall below the viewport at radius 56; the settled
   frame shows the compact four-corner marker around the fixture actor.
5. **Does the effect add unrequested sound or replace old art?** No. The
   Hound effect uses the transparent extracted frames and requests no sound.
   The map/HUD remain visible through all three phases and after release.

These captures verify the port's connected state and whole-screen composition;
they are not independent Genesis proof for target acceptance or creature
attacks.

## C0A2 target-action pose and cue review

ROM `0xB1DC` starts a linked-target action after the target and delay gates
accept it. It sets delay mode 1 through `0xABFC` (the source halves the
computed delay), requests cue `0x4C` for ordinary kinds or `0x55` for Hell
Hounds, selects the directional finite list through `0xC0A2`, and resets the
actor animation accumulator/frame at `0xAF6C`. The actor's `0x1514E`
initialization supplies threshold `0x1000`; `0x5FDC` adds the finite-list
increment on render heartbeats and `0x600E` holds the last finite-list frame.
The action resolver at `0x55C76` is called after this transition and is still
not connected to live encounter actors.

The offline importer now exports all directional action frames as individually
named actor assets and writes `actor-action-profiles.tsv` with the source kind,
frame count, increment, threshold, and cue. The extracted lists contain two
frames at `0x0200` for kinds 4-11 and 14, one frame at `0x01F4` for kind 12,
and three frames at `0x0200` for kind 13. Lua owns the finite-list accumulator,
completion state, transition condition, and cue request; C++ supplies that
named profile and renders the frame returned by Lua.

The Scene 2 field QA repositions the already-selected live actor only after
the natural offscreen-spawn and B-edge captures, then records both action
frames. All 14 random-encounter and 8 Hound BMPs are 320x224 and match between
Debug and Release by SHA-256. I reopened each full frame: the action frames
retain the full map and HUD, the visible actor does not cover an old control,
and no rectangular black background appears around its sprite. The B-edge
image is captured before the diagnostic reposition, so it correctly contains
no visible creature at the source edge-spawn position; the later settled
selector frame shows the repositioned actor. This is internal rendering and
state evidence, not an independent Genesis action-animation comparison.

The source makes one `0x4C`/`0x55` request at the transition, and the QA
observes that the B press/release is silent before the ordinary actor requests
`0x4C`. Audio playback now maps `0x4C` to extracted table-2 resource 76; its
FLAC contains 98,511 samples at 44.1 kHz (2.234 seconds), and the audio QA
checks that it loads, starts, and reports a duration in the expected range.
The `0x55` request uses table-2 slot 85, which is absent from the extracted
non-silent resources, so the host leaves it unmapped. The Hound's close-effect
diversion remains separate. No independent matching Genesis action sequence
has been identified. The damage/status result, actor retirement, and optional
child action after `0x55C76` remain open.

## Eight-question target-action review

1. **Does it look correct?** Both extracted ordinary action frames appear as
   the actor's directional sprite on the existing field scene. Full screenshots
   show no black tile rectangle or HUD damage. There is no same-actor Genesis
   sequence to confirm the exact pose timing.
2. **Does the look reflect the behavior?** Lua enters `action` only when a
   dynamic encounter actor has an accepted target, a negative action delay, a
   valid `C0A2` profile, and no active target-motion effect. The QA sees both
   frames and the one ordinary action-cue request.
3. **Are the visuals source-backed?** The importer reads the directional
   `C0A2` list and its finite frame count/increment from the ROM, exports the
   named frames with source palette and origin, and passes them to the renderer.
   The exact output is not called Genesis-verified.
4. **What animates, and when?** The initial frame is selected on `0xB1DC`;
   `0x5FDC` advances after the `0x1000` accumulator threshold, and `0x600E`
   holds the last finite frame until another behavior changes the list. The
   heartbeat follows the render pass, not a separate timer.
5. **What sound accompanies it?** The ROM requests `0x4C`, or `0x55` for kind
   12. Ordinary `0x4C` plays resource 76 once per transition; the decoded file
   lasts 2.234 seconds and the audio QA verifies playback. Slot 85 for `0x55`
   is verified silent/no-op and remains unmapped.
6. **Were neighboring regions checked?** Every full-screen encounter and Hound
   frame was reviewed. The right HUD is unchanged, the map remains intact, and
   no prior UI is drawn over by the creature sprite. The natural encounter
   actor remains offscreen in production-position screenshots.
7. **Are there multiple frames/examples?** Yes: both C0A2 ordinary action
   frames, two walk poses, selector transition/settled frames, production
   encounter checkpoints through 1,200 service passes, and all Hound effect
   checkpoints were reviewed. Longplay evidence corroborates selector brackets
   but not this exact action animation.
8. **Are new graphics covering stale ones?** The actor frame is a transparent
   named sprite composited by the field renderer. The HUD crop stays byte-for-
   byte identical to the baseline. No extra button, menu, or stale action
   graphic appears in the reviewed frames.

## Hell Hound close-effect review

1. **Does it look correct?** The source descriptor and scene palette produce
   a small directional flame effect around the Hound. The full 320x224 captures
   show the field map and HUD without a black rectangle around the flame.
2. **Does the look reflect the behavioral state?** Lua requests an effect only
   below distance `0x300` after signed `+0xDC` becomes negative. The host then
   reserves the slot returned by Lua; the test verifies slot 0 in the Lua
   contract and slot 2 after the two sequence actors occupy slots 0 and 1.
   Template delay 8/2 yields 90 behavior passes before the next effect.
3. **Are the visuals correct?** The three phases and eight headings are
   extracted from ROM effect descriptors with each field scene's palette.
   The renderer consumes the resulting named BMPs; it does not rebuild a VDP
   effect at runtime.
4. **What animates, and when?** `0x4E86` advances phases on the render
   heartbeat. Each visible phase lasts seven passes; phase four releases the
   effect without drawing. In this diagnostic placement the plume overlaps
   nearby field artwork. The full frames show each phase and the actor's
   continuing animation; the exact object/effect depth order is not confirmed
   by an independent capture for this encounter.
5. **What sound accompanies it?** Neither the `0x14C7C` close-attack branch
   nor its `0x4E86` phase path requests sound. The QA observes no audio request;
   there is no source duration to reproduce for this effect.
6. **Were neighboring areas checked?** The full-screen captures preserve the
   whole map and right-side HUD. The HUD remains pixel-identical across all
   phases. The effect overlaps scene artwork in the diagnostic image; because
   there is no matching independent Genesis sequence, the object-depth result
   remains unverified. Since the Hound keeps animating, the QA does not claim
   full-frame equality at release or use it to prove absence of stale pixels;
   phase four does clear the shared effect slot.
7. **Are there multiple frames and examples?** Yes: the QA captures encounter
   entry, the close-target state, effect start, all three source phases, and
   release. The reviewed longplay references for this hill show target
   brackets but do not provide a confidently identifiable Hound close-attack
   sequence, so they are not claimed as independent effect evidence.
8. **Are new graphics covering stale graphics?** The effect uses transparent
   source frames and is removed when phase four releases its slot. The HUD
   stays unchanged. The effect's overlap with nearby map artwork remains an
   open visual check because a same-encounter source frame is unavailable.

The QA-only placement makes the otherwise offscreen ROM encounter visible. It
does not establish a natural movement or attack route.

## Status

ROM-selected spawn, entry state, Lua target acquisition, close-range movement,
walk animation, the Hell Hound's source-timed close effect, and the B selector's
typed state and source pixels are covered
by focused Debug and Release QA. B's source-pool selection policy and the
ordinary gate's blocker/cadence/chance branches are covered by script-runtime
QA; the field spawn QA still injects the already-bounded selector rolls, so
production RNG consumption and event dispatch are not covered end to end. This
hill remains partial: the alternate target action-byte route through `+0x9C`,
the full actor pool, creature item/spell actions, hit resolution, damage/death,
and natural approach from the edge are unresolved. The independent longplay
corroborates the marker shape, but does not provide the actor bytes needed to
verify every palette variant or the Hound close-effect sequence.
