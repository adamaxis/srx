# Scene 2 Stoker collision candidate audit

## Result

The tested New Game route reaches the west collision margin of scene 2's ROM
terrain record 2, but it publishes field event `0` and opens no Stoker dialog.
Record 2 is a type-2 collision rectangle at `(0x560..0x57F, 0x340..0x34F)`;
that byte is not an NPC identity or a dialog event number. It is far east and
south of the motel doorway shown in section 03, so this candidate is rejected
as the Stoker trigger.

The visible motel doorway maps to scene-2 terrain record 91 at ROM `0x173138`:
`(0x3C0..0x3DF, 0x280..0x29F)`, value `0xC2`. The ROM bytes were checked
directly. New Game starts at `(0x3CE0, 0x2D00)`; normal Up input reaches this
door's collision margin at `(0x3CE0, 0x2A58)` and the current runtime loads
scene `0x14` through event-table row 0. It still does not open the Stoker page
through 30 neutral destination updates. The independent clip shows the same
door approach in section 03 frames `000028..000054`, a fade at `000055`, a
blank generic panel at `000080`, and Stoker's populated panel at `000082`.
Those frames establish the visual sequence, but do not expose controller
input or the event state. The physical property/state that selects mode 2,
event 2 remains unresolved, so this audit does not defend or resolve Stoker's
live collision route.

ROM `0x1F65A` selects the Stoker profile and `0xF4AA0..0xF4B2C` prompt graph
only after the field-mode/event tuple is already mode 2/event 2. The candidate
route never reaches that selector. The Qt dialogue QA that injects this tuple
is a separate dialogue-conversion test; it is not evidence of physical
contact. The ROM location or property that produces the tuple is still
untraced.

## Source route and result

- New Game begins in scene 2 at raw fixed-point position `(0x3CE0, 0x2D00)`.
- Ordinary D-pad movement follows the open street, routes around the wall, and
  approaches scene 2 collision-object record 2 at ROM `0x172DBE`.
- Record 2 covers world X `0x560..0x57F`, Y `0x340..0x34F`. The movement
  collision margin makes the final attempted position `(0x5540, 0x3480)`—
  world position `(0x554, 0x348)`—a physical contact with the west edge.
- The source collision/movement path at `0xB7B8` leaves the result at scene 2,
  event `0`, projected code `0`; no dialogue, interaction modal, or transition
  starts.
- A separate New Game route with normal Up input publishes event 0 and selects
  destination scene `0x14` from scene 2's event table. Its current Qt QA keeps
  dialogue closed through 30 neutral destination updates. This proves that
  scene-entry path only; it is not evidence that record 2 opens the Stoker
  prompt or that event 0 is the Stoker event.
- The separate scene-2 terrain record 91 (`0x173138`, value `0xC2`) bounds the
  motel entrance. The New Game Up route reaches its collision margin and
  publishes event 0 before loading scene `0x14`; the current route does not
  open Stoker dialogue.
- Independent section 03 frames `000028..000054` show the same doorway
  approach, `000055` begins the fade, `000080` is a blank generic panel, and
  `000082` shows the populated Stoker panel. No controller log or collision
  coordinate is present in that recording.

The Qt CTest uses only held and released D-pad input for this route. At the
south wall, it also verifies that a blocked held step resets the actor to
C052 idle with frame index zero. Debug and Release both pass. Each run writes
22 full-screen 320x224 BMP checkpoints to
`build-qt-runtime-{debug,release}/qt_runtime/field-stoker-collision-qa/`:
spawn; eastward movement and waypoint; southward wall approach; wall bypass;
the property lane; the final contact candidate; idle frames 1, 8, and 30; and
`actual.bmp`. All 22 Debug/Release filenames and SHA-256 hashes match. The
frames were reviewed at full-screen size; the map and HUD remain visible and
no dialogue/menu controls are overlaid.

## Eight-question review

1. **Does it look correct?** The full captures show a continuous scene-2 route
   and the expected event-0/no-dialogue outcome. No matching independent
   Genesis capture of this same object or route was identified, so exact
   object appearance and placement are not independently verified.
2. **Does the look match behavior?** Yes for the tested conversion state: the
   actor reaches the record-2 collision margin, stays in scene 2, retains
   event/projected code 0, and opens no modal. The state is checked after the
   contact and during 30 idle updates. At the separate wall row, normal held
   input is rejected and the typed selector/frame return to C052 idle/zero.
3. **Are the visuals themselves correct?** The full map/HUD composition is
   internally stable through the route. Exact source pixels at this property
   are unknown because there is no matching independent frame. The moving
   actor pose is also being reviewed under defect goal `FIELD-01`; this route
   does not declare it correct.
4. **Is it animated?** The field actor animates during held movement. At the
   wall, a blocked step returns to a static C052 pose; idle checkpoints at
   frames 1, 8, and 30 keep the same position. The route does not exercise a
   Stoker/NPC animation or identify the actual property trigger. The source
   cadence and other live headings remain part of `FIELD-01`.
5. **Is there a sound?** This audit does not identify a collision-specific
   source sound or its lifetime. The CLI's accumulated audio-event count is
   diagnostic only; it does not prove playback or prove the absence of a
   sound.
6. **Are other screen areas correct?** The full-screen review includes the
   complete playfield and right HUD at all 22 checkpoints. No stale page,
   button, dialogue frame, or duplicate HUD layer appears. The portrait-face
   palette issue remains open under HUD-01; this collision test does not
   validate that portrait or the visible ammo counts. The actor animation
   concern is tracked separately rather than hidden by this collision result.
7. **Do multiple frames support the interpretation?** Yes internally: route
   checkpoints cover movement, wall rejection/bypass, contact, and idle. The
   independent section 03 frames `000028..000055` show the doorway approach
   and fade, and frames `000080..000082` show the blank and populated Stoker
   pages. They do not show controller input or reveal the collision state, so
   they cannot identify what selects event 2. Section 04 frames
   `000900..000911` show other original-game field states and do not depict
   the scene-2 candidate.
8. **Was prior UI cleared?** No UI is opened on the tested candidate route.
   Freshly rendered full frames contain no stale dialogue/menu controls.

## Outcome labels

- **Code changed:** a reproducible negative-candidate QA route and named CTest
  were added. The route clears stale BMPs in its own output directory and
  verifies the exact 22-file output set.
- **Behavior implemented:** no new Stoker behavior was implemented. The QA
  distinguishes the unrelated east/south record 2 from the visible motel
  doorway at record 91. Normal Up input contacts record 91, publishes event 0,
  and reaches scene `0x14`; the independent clip proceeds to Stoker dialogue,
  but its input/event state is not observable.
- **Internally checked:** `shadowrun_ghidra_conversion_qt_field_stoker_collision_candidate_qa`
  passes in Debug and Release; it verifies the wall reset to C052/frame zero,
  and all 22 full-screen outputs match by SHA-256 and were reviewed.
- **Independently visually checked:** section 03 shows the city-door approach,
  fade, and Stoker prompt on successive full frames. It does not expose input
  or event state, so it cannot yet validate the native contact behavior.
- **Reported defect resolved:** no. The actual physical mode-2/event-2
  property and its collision-to-dialogue path are still open.
