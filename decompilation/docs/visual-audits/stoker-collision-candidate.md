# Scene 2 collision record 2 negative-control audit

## Result

Scene-2 terrain record 2 at ROM `0x172DBE` is not the Stoker doorway. Its
rectangle is `(0x560..0x57F, 0x340..0x34F)`, far east and south of the motel.
The tested New Game route contacts its west collision margin at raw position
`(0x5540, 0x3480)` and world position `(0x554, 0x348)`. The field remains in
scene 2, event 0 is published, and no dialogue or transition begins.

The actual doorway trigger is scene-2 record 91 at `0x173138`, bounds
`(0x3C0..0x3DF, 0x280..0x29F)`, value `0xC2`. Its physical collision-to-Stoker
dialogue path is traced and tested separately in
[`motel-doorway-scene-entry.md`](motel-doorway-scene-entry.md). That route
publishes raw collision `0xFFC2`, selects mode 2/event 2, and opens Stoker's
generic arrival dialogue. Record 2's event-0 result is not evidence for that
positive route.

## Negative-control route and QA

- New Game starts in scene 2 at `(0x3CE0, 0x2D00)`.
- Ordinary D-pad movement routes east and south to record 2's expanded west
  collision margin. The blocked step leaves the actor at `(0x5540, 0x3480)`.
- The source collision path at `0xB7B8` returns event 0 for this record. The
  scene stays 2 and the dialogue modal remains closed through 30 neutral
  destination updates.
- At the separate wall row, held movement is rejected and the connected ROM
  `0xB442` reset returns to C052 idle, frame 0. A subsequent held diagonal at
  the corner reaches the same live reset callback; the candidate position is
  unchanged, and releasing input restores the desired heading and idle pose.
- The independent section 03 recording shows the motel approach and populated
  Stoker panel, but does not show record 2, its collision coordinates, or the
  event state. It therefore cannot independently validate this negative
  control.

The candidate CTest writes 23 full-screen 320x224 BMP checkpoints covering
spawn, movement, wall approach, corner reset/idle, bypass, the property lane,
contact, idle frames 1/8/30, and `actual.bmp`. Debug and Release output names
and SHA-256 hashes match. All frames were reviewed at full size; the playfield
and HUD remain visible, with no dialogue, menu controls, or duplicate UI layer
overlaid.

## Eight-question visual review

1. **Does it look correct?** The captures show the continuous scene-2 route
   and the expected no-dialogue outcome. No matching independent Genesis frame
   of this record-2 contact was found.
2. **Does the look match behavior?** Yes for this negative control: contact
   leaves scene 2 active with event 0 and no modal. Blocked wall and corner
   movement run through the connected B442 reset and return to the source-derived
   idle selector and frame.
3. **Are the visuals correct?** Whole-screen captures show a stable field and
   HUD during movement and idle. Exact source pixels at record 2 are not
   independently available; FIELD-01 and HUD-01 remain separate open goals.
4. **Is it animated?** The runner changes poses during held movement and is
   static at the blocked wall and idle checkpoints. The QA confirms the
   directional reset frames after both wall and diagonal corner rejection.
   This route does not test a Stoker animation or the positive doorway sequence.
5. **Is there a sound?** This negative-control route identifies no
   collision-specific source request or duration. Its event count cannot prove
   playback or silence.
6. **Are the other screen areas correct?** The full-screen review includes
   the complete field and HUD. No stale dialogue, menu, or duplicate controls
   appear. It does not verify portrait palette or ammunition values.
7. **Do multiple frames support the interpretation?** Native checkpoints cover
   movement, the blocked wall, corner reset, contact, and 30 idle updates. Independent
   section 03 frames show the separate Stoker doorway sequence, not this
   record-2 route or its event result.
8. **Was old UI cleared before drawing new UI?** No UI opens on this route, and
   all full-screen captures remain free of stale dialogue/menu content.

## Outcome

- **Code changed:** a reproducible record-2 negative-control route and CTest
  distinguish this terrain object from the Stoker doorway; the connected live
  field movement reset now handles blocked corner probes.
- **Behavior checked:** record 2 contact remains scene 2/event 0 and opens no
  dialogue. The positive record-91 route is covered by the linked audit.
- **Internally checked:** the candidate and field-movement CTests pass in Debug
  and Release; all 23 full-screen outputs match by SHA-256 and were reviewed.
- **Independently visually checked:** the supplied recording does not show
  this candidate object, so no independent visual claim is made for record 2.
- **Reported defect resolved:** this file is only a negative control. The
  positive Stoker route's result and evidence are recorded in the linked audit.
