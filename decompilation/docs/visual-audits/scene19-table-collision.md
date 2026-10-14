# Scene 19 table collision review

**Status:** Source-backed contact QA passes; no host collision change is
warranted. The first candidate touching each table record is rejected, and
the final committed position stays on the approach side. The ROM rectangles
are left unchanged.

## Reproduction and source

Run Release CTest `shadowrun_ghidra_conversion_qt_field_movement_qa`. It
bootstraps the ordinary field runtime on Scene 19, approaches collision
records 8 and 9 from clear map cells, and saves the first blocked candidate.
The full 320x224 captures and per-tick evidence are under
`build/conversion/qt/release/qt_runtime/field-movement-qa/scene19-table-contact/`.
Each route has `entry.bmp`, `contact-before.bmp`, `contact-after.bmp`,
`actual.bmp`, and `terrain-probes.tsv`; the parent `contacts.tsv` collects the
source record, candidate, four corner samples, and committed coordinates.

The US ROM bytes are the primary collision evidence:

| Scene record | ROM address | Bytes | Rectangle | Value |
|---|---:|---|---|---:|
| 8 | `0x1D5F28` | `00 40 00 7F 00 80 00 9F 00 7E` | `(64..127,128..159)` | `0x7E` |
| 9 | `0x1D5F32` | `00 A0 00 DF 00 80 00 9F 00 7E` | `(160..223,128..159)` | `0x7E` |

ROM `0xEE10` rasterizes these inclusive rectangles on the 32-unit terrain
grid. `0xE7C8` treats `0x7E` as blocked. Movement candidates and the
four-corner offsets `(-12,-7)`, `(+12,-7)`, `(-12,+7)`, and `(+12,+7)` follow
the `0xBABC` / `0xB77E` / `0xE8A0` path; the Lua movement policy handles its
source fallback before the accepted position is committed.

The first QA attempt spawned at `(16,140)`, inside Scene 19's separate ROM
`0x7F` boundary rectangle `(0..31,96..255)`. That run failed before reaching a
table and did not show a table collision defect. The approach starts were
moved to clear source cells. The enclosing movement QA also now gives each
independent movement runtime the same seeded random-event miss and world
counter, so its expected 16-frame route cannot be changed by a world-activity
cadence point. These are test setup corrections; no gameplay collision code
changed.

## Contact results

| Route | Blocked candidate | Blocked corner(s) | Position after the update |
|---|---|---|---|
| Record 8 west | `(53,132)` | 3 | `(51,132)` |
| Record 8 east | `(138,130)` | 2 | `(140,130)` |
| Record 8 north | `(118,121)` | 2 | `(120,117)` |
| Record 8 south | `(96,166)` | 0, 1 | `(96,168)` |
| Record 9 west | `(149,132)` | 3 | `(147,132)` |

Coordinates above are normalized map units; the TSV keeps exact fixed-point
values. Every blocked candidate samples the expected `0x7E` and returns
`0x7F`. The resulting position stays outside the corresponding ROM rectangle
on the approach side. Some updates also move along or away from the edge under
the ROM movement fallback, so the test checks the rejected axis against the
rectangle instead of incorrectly requiring the entire position to be frozen.
The first clear approach cells contain `0x00`; the border rectangles remain
visible in the probe traces as `0x7F` where applicable.

## Visual review

1. **Does it look correct?** The complete Release frames show the player
   approaching the two static pool tables. The tables, orange strips, floor,
   and HUD remain present in every route.
2. **Does the look reflect behavior?** Yes for the captured contact. The
   sampled candidate reaches the source table record, returns blocked, and
   the committed runner remains on the same side of that rectangle.
3. **Are the visuals correct?** The extracted table and wall-strip placement
   matches the longplay map view. Collision remains source-defined even where
   the rectangle includes dark pixels read visually as a shadow.
4. **Should it animate?** The tables and wall strips are static map art. No
   table animation record is associated with them.
5. **Is there a sound?** These contact routes emitted no audio requests. This
   says nothing about unrelated room music or ambient audio.
6. **Were nearby screen elements reviewed?** Yes. All five contact-after
   frames were inspected at full 320x224 size, including the runner, both
   tables, wall strips, floor, portrait, inventory slots, ammo labels, and
   remaining HUD. No modal residue or unrelated graphic changed during the
   movement route.
7. **Do multiple frames support the reading?** The host approach/contact pairs
   provide consecutive movement evidence. Independent longplay section 09
   frames `006990` and `007035` show the same tables at different player
   positions. They support static placement and ordinary movement, but do not
   show these exact collision contacts or prove ROM controller timing.
8. **Were new controls drawn over old UI?** No controls are created by this
   field movement path. The existing HUD remains visible; no buttons or menu
   layers are drawn over it.
9. **Was the full time window reviewed?** The QA records the approach and the
   first blocked candidate for each route. The 30-fps longplay samples are
   reference anchors, not a full VBlank or collision-input trace.

## Test and limits

The first test run failed because its Scene 19 spawn was already inside a
source `0x7F` wall. After moving the test start to a clear cell and keeping the
movement fixture's world tick deterministic, Release CTest passes. This is a
source-confirmed no-fix review, not a failing-then-passing engine behavior
change. The independent longplay does not show the exact contact frames, so
this does not claim full Genesis visual or controller-path verification.
