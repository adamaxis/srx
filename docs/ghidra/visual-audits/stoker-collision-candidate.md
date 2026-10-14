# Scene 2 collision record 2 and obstacle-edge route

## Result

Scene-2 terrain record 2 at ROM `0x172DBE` is not the Stoker doorway. Its
inclusive rectangle is `(0x560..0x57F, 0x340..0x34F)`, far east and south of
the motel. The tested New Game route reaches a blocked candidate at fixed
position `(0x5540, 0x3534)` from committed position `(0x5522, 0x3534)`.
Normalized candidate `(0x554, 0x353)` puts the expanded right probes exactly
on the record's west edge. The field remains in scene 2, event 0 is
published, and no dialogue or transition begins.

The actual doorway trigger is scene-2 record 91 at `0x173138`, bounds
`(0x3C0..0x3DF, 0x280..0x29F)`, value `0xC2`. Its physical collision-to-Stoker
dialogue path is traced and tested separately in
[`motel-doorway-scene-entry.md`](motel-doorway-scene-entry.md). That route
publishes raw collision `0xFFC2`, selects mode 2/event 2, and opens Stoker’s
generic arrival dialogue. Record 2’s event-0 result is not evidence for that
positive route.

## ROM trace and connected movement

The scene-2 wall route exercises the ROM’s actual diagonal fallback policy:

- `0xB77E` probes a direction and leaves the candidate coordinates in D5/D6
  while recording the direction slot at `FFDF66`.
- `0xBADE` detects the blocked diagonal candidate and calls the policy probe
  at `0xB732` for an orthogonal side direction. A clear first probe commits;
  if it is blocked, the second side direction is tried.
- When either side is clear, `0xBB70` copies the last probe direction from
  `FFDF66` to entity `+0x36`, sets the source movement flag, and falls through
  to `0xBBE4` to commit that same probe’s D5/D6 coordinates.
- `0xB442` is reached when both side probes remain blocked. A blocked aligned
  route also resets through this handler.

The typed conversion now carries the full `MovementProbeResult` through both
the aligned route and the diagonal side probes. `MovementRouteResult` retains
the aligned fallback candidate; the diagonal step commits the final side
probe, including its source direction slot. The Qt field controller supplies
the real map collision evaluator to this path.

## Connected route and full-screen QA

The QA starts from the New Game scene-2 spawn `(0x3CE0, 0x2D00)`, follows
ordinary east/south input to the wall, attempts the diagonal input, releases
input for 30 consecutive frames, bypasses the wall, then contacts record 2.
At the wall, the diagonal candidate is blocked but one orthogonal probe is
clear. The live controller commits that cardinal candidate instead of
resetting the actor in place. The final seven neutral frames verify that the
position has settled. A separate held movement into the wall with no usable
path leaves the actor at the source-derived C052 idle pose.

Record 2 contact remains scene 2/event 0 with the dialogue and interaction
windows closed. The candidate CTest writes 58 full-screen 320x224 BMPs,
including the pre-contact wall frame, every frame in the diagonal attempt,
all 30 neutral release frames, bypass frames, the record-2 approach, the
before/after contact pair, and final idle frames. Debug and Release contain
the same filenames and SHA-256 hashes. All 58 Release frames were inspected
at full size; the Debug images are byte-identical.

The contact trace is at
`build/conversion/qt/release/qt_runtime/field-stoker-collision-qa/` and the
matching Debug directory. `property-contact-probes.tsv` records candidate
`(21824,13620)` fixed, normalized `(0x554,0x353)`, with corner bytes
`0,2,0,2`, raw terrain `0x02`, and return `0x007F`. The committed position
stays `(21794,13620)` fixed. ROM routine `0xB574` selects movement vectors;
for this calm on-screen runner, the ROM table at `0xBEC2` advances east by
`+0x001E` fixed units. This is why the candidate is `0x5540`, not a `0x28`
step. `0xE8A0` samples the four corners, and `0xE9EE` checks the type-2
record bounds. The rectangle is the source collision authority; no gameplay
collision change is warranted.

The QA first failed after it was tightened to require a live record-2
contact: its trace reader used an accumulating-list index even though the
runtime clears probes each update, and its assertion assumed `+0x28` rather
than the ROM's `0xBEC2` `+0x1E` step. The test instrumentation and expectation
were corrected; no product collision behavior was changed. The Release
before/after contact frames are `property-contact-before.bmp` and
`property-contact.bmp` under the path above.

The independent section 03 recording shows the motel approach and populated
Stoker panel, but it does not show record 2, this wall, the collision
coordinates, or the event state. It cannot independently validate this
negative-control path.

## Nine-question visual review

1. **Does it look correct?** The full-screen frames show a coherent scene-2
   field route. No independent Genesis frame for this wall or record-2
   contact was found, so appearance is internally reviewed but not claimed as
   an independent match.
2. **Does the visible look match behavior?** Yes for this route: the field
   stays in scene 2, event 0 remains active, record 2 opens no dialogue, and
   the diagonal obstacle contact advances through the clear cardinal fallback
   rather than falsely invoking the blocked-step reset.
3. **Are the visuals correct?** The complete playfield and HUD remain drawn
   through movement and neutral release. No clipping or collision-induced
   graphics change appears in the captures. Exact Genesis pixels for this
   object remain unavailable.
4. **Is it animated?** The player and map update across the obstacle contact.
   The ROM movement code selects the fallback direction and the shared field
   controller continues the movement; no new animation was added. The
   consecutive 30-frame release capture includes the settle interval and
   final stable position.
5. **Is there a sound?** The traced `0xB6DE`, `0xBADE`, `0xBB70`, `0xBBE4`,
   and `0xB442` movement path issues no sound call. The connected QA records
   zero sound events for this route. Other field movement audio remains an
   open goal; no duration is inferred here.
6. **Are the other visible parts of the screen also correct?** All frames
   include the full field and HUD. The portrait, status column, and nearby
   map remain visible while the camera follows the actor.
7. **Do multiple frames support the interpretation?** Yes. The wall
   approach, diagonal input, release, bypass, contact, and idle have separate
   full-screen samples; Debug and Release images match byte for byte. The
   contact pair shows the live runner immediately before and after the
   rejected candidate. The supplied independent clip does not contain this
   collision route.
8. **Does the new UI replace the old UI cleanly?** This route does not open
   a menu or dialog. No old menu controls or graphics appear over the field.
9. **Was the affected time window reviewed?** The wall approach, every
   diagonal-input frame, and the following 30 neutral frames were captured
   consecutively and inspected. Earlier east/south travel is represented by
   checkpoints rather than a continuous capture; it is not part of the
   collision decision window. The final route holds at the destination long
   enough to verify a stable field state.

## Outcome

- **Code changed:** native route and step results preserve the final source
  collision probe so the connected controller commits its actual fallback
  coordinates and direction.
- **Behavior implemented:** this one-side-clear obstacle edge now slides
  along the valid orthogonal route; fully blocked movement still resets.
- **Internally checked:** the route, field-movement, and collision-to-dialogue
  CTests pass in Debug and Release. Movement route/step contract tests pass in
  both configurations. All 58 screen dumps match across Debug and Release;
  the record-2 probe and retained position are asserted directly.
- **Independently visually checked:** no matching real-game frame exists for
  this wall/record-2 path. The separate Stoker doorway frames do not prove it.
- **Reported defect resolved:** the tested one-side-clear edge case no longer
  sticks at the blocked diagonal candidate. FIELD-02 remains partial because
  broad terrain recovery and other collision/property routes are still open.
