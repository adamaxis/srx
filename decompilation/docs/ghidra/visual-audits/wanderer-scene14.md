# The Wanderer (field scene 14)

Status: partial. The map viewer now models the room's named fixtures and the
scene-14 event-30 dialogue profile. The first-visit intro, conditional credit
root, and post-Ito booth-vacated branch use the shared Lua Johnson interaction.
The departure branch is physically exercised in Debug and Release. The
work/quest choice effects remain incomplete. The lower event-0 transition to
field scene 0 is source-traced and physically exercised.

## ROM trace

- Scene 1 event record 73 at `0x17D9DE` is event 0. Event-table row 0 at
  `0x17D620` selects numeric field scene `0x0E`. ROM contract location
  `[1][0]` names the Wanderer; Mike's Puyallup map labels marker 8. The display
  name is a cross-reference, while the route byte itself is only scene 14.
- Scene 14's default actor-list record is empty. The extracted source actor
  census also has no conditional pedestrian candidates for scene 14. Event-30
  dialogue uses portrait art and does not add a standing NPC to the room.
- Scene 14 event record 9 at `0x1D5C10` is raw `0x019E`, event 30, at map
  pixels `(256,32)` through `(287,63)`. ROM `0xB7B8` dispatches event 30 to
  `0x20990`; the scene-14 branch sets dialogue slot 3 in `FFF0AE` and sets
  `FFFC27` bit 3.
- The normal dialogue row is row 3 at `0x12CC80`: portrait profile `0x121F28`,
  first-visit descriptor `0x1052BC`, and ordinary menu descriptor `0x105300`.
  When `FFFC27` bit 6 is clear, the handler sets it and selects the first-visit
  path. Text `0x106D7C` describes the suited Johnson and bodyguard; `0x106E5E`
  introduces Vigore and Jarl; `0x10702D` greets Joshua and opens the
  information/work/leave menu. The shared Lua `johnson.interaction` now shows
  the two first-visit pages once per known fresh-game state, then uses the
  location's script-defined menu labels.
- The shared conditional descriptor gate at `0x20AFC` selects descriptor
  `0x105988` when `FFFBEC` is negative, `FFFC1C` bit 7 is set, and the low
  byte of `FFFBEC` matches dialogue slot 3. Its opening text at `0x105BD9`
  says Vigore owes credits. The Wanderer profile selects the conditional
  opening root for a completed context-3 Johnson return; Lua route QA covers
  that root.
- The scene-14 branch checks `FFFC1E` bit 16 before the first-visit flag and
  ordinary menu. If the low byte of `FFFBEC` is 3, it calls `0x1E68C`, which
  sets `FFFBEC` to `-1` and clears `FFFC1C`. It then uses blank profile
  `0x121E20` and descriptor `0x105370`; text `0x10760C` says the booth Vigore
  and Jarl used is empty. The descriptor's first-choice field is
  `0xFFFFFFFF`, so the ROM presents one closing page with no menu. The nearby
  `0x10742E` fixer text is behind that sentinel and is not shown on this
  branch. Lua now applies the matching context-3 reset and uses a narrator
  profile with no portrait.
- Scene-14 event records 7 and 8 (`0x1D5BFC`, `0x1D5C06`) are adjacent
  event-0 cells at the lower map edge, with no event table. The profile pointer
  at `0x1D5BA0` is null; the generic `0x167AE` selector reads ROM byte 0 and
  chooses field scene 0. The shared Lua location-link resolver now represents
  that static source result. It does not infer a return to Puyallup, a dynamic
  prior-map destination, or a special exit position or heading.
- Debug and Release physical-input QA cases start one cell north of each
  rectangle, enter record 7 or record 8 with ordinary Down, and complete the
  scene-0 transition. Each case saves full 320x224 approach, event-contact,
  and destination captures plus a route manifest. The contact capture is the
  first full-screen black fade frame; `route.tsv` records the contact position,
  and `actual.bmp` shows the completed scene-0 destination. These captures
  verify the host path; no independent Scene-14 gameplay frame has been
  identified.
- `shadowrun_ghidra_conversion_qt_field_wanderer_vacated_entry_qa` starts one
  cell south of record 9, sets `FFFC1E` bit 16, and enters event 30 with
  ordinary Up input. It checks the closing text, blank portrait well, and zero
  choice rows, then writes full-screen `spawn.bmp`, `event-contact.bmp`, and
  `actual.bmp` plus `route.tsv`. Debug and Release outputs match by SHA-256;
  both full 320x224 `actual.bmp` frames were inspected. No independent Genesis
  frame of this Scene-14 dialogue appears in the supplied longplay, so these
  captures verify the host route and layout only.

## Map entities

The authoring map groups the back-bar shelves, counter, attached wall display,
and priority-plane rails into one bar entity. It separately names the raised
Vigore/Jarl booth, both pool tables, three lamp-table/seat groups, two square
lamp fixtures, and two east-wall metal fixtures. Composite and priority crops
stay attached to the same object where appropriate. No collision is assigned
from appearance alone. Floor texture remains part of the map background.

## Seven-question visual review

1. **Does it look correct?** The named parts follow the extracted Scene 14
   composite and priority image. The green felt objects are called pool tables
   because the map pixels show cues and scattered balls. The east-wall pieces
   are called metal fixtures because their purpose is not identified by the
   source text.
2. **Does the look reflect behavior?** The event-30 cell overlaps the raised
   booth, and the source dialogue names Vigore and Jarl. Their portrait is
   dialogue art. The default actor list is empty, so this map gets no invented
   resident sprite.
3. **Are the visuals correct?** The entity parts use the ROM-extracted
   composite and priority planes. They annotate the map; they do not replace
   its pixels. The portrait `0x121F28` was decoded to a 56x72 image. The
   conditional `0x121E20` branch decodes to a blank image area.
4. **Is anything animated?** No fixed actors or animated map fixtures are
   present in the scene's default record. Random world-event sequences are a
   separate runtime system and are not placed as residents here.
5. **Is there sound?** No scene track or event-specific sound request has
   been traced for this route. The Map audio panel keeps that evidence
   unresolved instead of calling the scene silent.
6. **Are other screen elements clean?** Entry and exit records, event-30
   interaction, default actors, dynamic world-event actor candidates, map
   artwork, and authored entity annotations remain separate. The lower event-0
   records are outside the visible map and are shown as small selectable edge
   markers instead of large rectangles over empty canvas. Their destination is
   now identified as field scene 0 by the null-table fallback.
7. **Do multiple frames support this interpretation?** A 1-fps scan of the
   extracted longplay folders found no strong Scene-14 map match. The best
   downsampled match scored `0.393`; a known Scene-13 frame matched its map at
   `0.945`. No Scene-14 video frame is claimed as visual corroboration. The
   room segmentation is therefore supported by ROM map pixels, not an
   independent gameplay sequence; dialogue pages need their own capture if a
   later recording includes the Wanderer.

The next pass should trace the work offer's selected run records and choice
effects. Naming the lower exit's world location or establishing a reverse
route still requires separate source evidence.
