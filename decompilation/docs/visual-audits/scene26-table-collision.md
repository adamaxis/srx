# Scene 26 table collision review

**Status:** Source behavior verified; no collision change is warranted for
record 70. The rectangle is ROM data and overlaps the visible table shadow.
The reported stop at that boundary is therefore expected. The full scene's
independent Genesis visual comparison remains open.

## Reproduction and evidence

Release CTest `shadowrun_ghidra_conversion_qt_field_scene26_event1_entry_qa`
now continues the ordinary scene-2 event-1 route into Scene 26, places the
runner on the visible north approach, holds Down, and saves the first blocked
step. The trace is at
`build/conversion/qt/release/qt_runtime/field-scene26-event1-entry-qa/table-collision/`:

- `table-contact-before.bmp` and `table-contact-after.bmp` are complete
  320x224 frames. Both retain the field HUD and show the runner beside the
  table.
- `actual.bmp` is the settled full-screen contact frame.
- `table-contact.tsv` records the committed position, attempted candidate,
  four source probes, selected tile, and result.
- `terrain-probes.tsv` contains every probe in the approach.
- `build/conversion/qt/release/qt_runtime/l1-scene26-table-rect-overlay.png`
  shows record 70 over the extracted map art. The object association is a
  visual inference; the ROM record itself is an unnamed collision rectangle.

No matching Scene 26 terminal/table frame was found in the longplay footage or
its reviewed contact sheets. The user's supplied captures establish the
reported table/shadow appearance, but have no longplay timestamp. No exact
longplay frame is claimed.

## ROM trace and host result

The US ROM bytes at `0x1D9C2A` are
`00 60 00 BF 00 E0 00 FF 00 7E`: record 70 is `(96..191,224..255)`, value
`0x7E`. The same ten bytes appear at index 70 in the extracted Scene 26
collision records. `0x0EE10` rasterizes the inclusive rectangle on the
32-unit grid.

At `0xB77E`, the ROM adds its directional vector to the actor position and
calls `0xB48C`; `0xB48C` calls terrain routine `0xE7C8`. The four-corner path
at `0xE8A0` samples offsets `(-12,-7)`, `(+12,-7)`, `(-12,+7)`, `(+12,+7)`.
For the captured attempted step, those probes are:

| Probe | Coordinate | Terrain byte |
|---|---:|---:|
| 0 | `(138,211)` | `0x00` |
| 1 | `(162,211)` | `0x00` |
| 2 | `(138,225)` | `0x7E` |
| 3 | `(162,225)` | `0x7E` |

The same host update then probes the retained position `(2408,3462)` again,
normalized to `(150,216)`. Its four terrain bytes are all `0x00` and its
collision result is clear. The before/after actor position remains unchanged.
Both probes from that update are retained in `terrain-probes.tsv`.

The Release host's candidate fixed position is `(2408,3492)`, normalized to
`(150,218)`; its committed position before and after the attempted step is
`(2408,3462)`. `0xE7C8` maps the sampled `0x7E` to blocked result `0x7F`.
The ROM's `0xBADE` movement routine sends nonzero collision results through
its movement-result/route policy (`0xB732` / `0xBB86`) before its later
commit/reset branches; it is not a direct early return for every blocked
sample. This QA verifies the ROM's terrain classification for the exact host
candidate and records the host's retained position; it does not claim a
hardware trace of the entire controller service. Ghidra C was used as a
cross-check; record bytes and collision classification were read from the US
ROM and extracted record.

The map overlay places the ROM rectangle over the teal table body and its
lower edge, then into some of the dark pixels visually read as shadow. This
is why the feet can appear to stop at the shadow even though the table body is
the solid object. The source rectangle controls this result; shrinking it to
the bright pixels would diverge from the ROM.

## Visual review

1. **Does it look correct?** The full frames show the runner at the table's
   lower edge, with the teal table, its black shadow, floor, and HUD present.
   The attempted movement does not push the actor through record 70.
2. **Does the appearance reflect state?** Yes. The recorded position remains
   `(2408,3462)` after the Down candidate is rejected by the two `0x7E` probes.
3. **Are the visuals correct?** The table and shadow are separate extracted
   map pixels. The ROM collision rectangle overlaps both table pixels and
   part of the shadow; that overlap is source-authored.
4. **Animation?** The table and shadow are static. No animation cadence is
   attached to this collision record.
5. **Sound?** The blocked table step adds no sound request in the traced
   movement path. No duration applies.
6. **Related screen areas?** Both contact frames were reviewed at full size;
   the HUD, side panel, floor, and map remain present. The renderer's sparse
   ROM-priority pixels remain a separate draw pass over the ordinary actor.
7. **Multiple frames?** The contact pair and approach trace show the accepted
   movement followed by the rejected step. These are host frames. The exact
   Scene 26 overlap has no matching independent longplay frame.
8. **Stale UI or graphics?** No new controls are drawn for this event. The
   full-frame captures show the expected field HUD and no modal residue.

## Test result and remaining work

The existing terrain probe already maps this candidate to the ROM's blocked
`0x7E` cell, so there was no terrain-classification failure to fix. The QA
now asserts its deterministic route's committed and candidate positions, all
four probe coordinates and bytes, the selected grid cell, and `0x7F` result;
it passes in Debug and Release. No artificial failing state or collision
adjustment was introduced. This is the handoff's explicit source-rectangle
stop condition, not a claim that the full controller movement path or map
hill is defended.

### L3 diagonal furniture edge and idle tail

The same `shadowrun_ghidra_conversion_qt_field_scene26_event1_entry_qa` now
captures a Down+Right approach to record 70 after aligning the ROM heading.
The deterministic route's full-screen frames are in
`build/conversion/qt/release/qt_runtime/field-scene26-event1-entry-qa/table-collision/diagonal-corner/`:
`entry.bmp`, `contact-before.bmp`, `contact-after.bmp`, `actual.bmp`, and
`neutral-01.bmp` through `neutral-30.bmp`. The complete probe and idle traces
are `movement.tsv`, `contact.tsv`, and `neutral.tsv`.

On update 10, the runner is at `(1390,3470)` fixed point. The diagonal
candidate is `(1420,3500)` and normalizes to `(88,218)`. Its four ROM terrain
samples are `00,00,00,7E`; record 70 at `0x1D9C2A` supplies the lower-right
blocked sample. Collision returns `0x7F`. The later axis fallback commits
`(1420,3470)`, preserving the clear X movement while rejecting the Y
component. This is the same source result in the typed host path, so no
movement fix was warranted. Thirty neutral updates keep the world position
fixed; the runner sprite's animation continues, and the route adds no audio
request or modal state.

The 320x224 entry/contact/settled frames were reopened at original size. The
field HUD stays in place, the runner remains visible at the table edge, and
the teal table is priority-drawn over the runner while the dark shadow remains
part of the base map. No extra control or stale panel is layered over the HUD.
There is no exact longplay frame for this Scene 26 contact, so these frames
review host composition only. The source collision rectangle and one-corner
sample are the behavioral evidence.

The broader object-bound census, Scene 19 table records 8/9, independent
Genesis visuals for Scene 26, and the other collision/menu rows remain open.
