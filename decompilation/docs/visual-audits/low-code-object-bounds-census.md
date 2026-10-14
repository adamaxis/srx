# Low-code object-bounds collision census

**Status:** Eight visibly recognizable map regions were probed against their
active ROM collision records. Six stop on the matching low-code record. The
two shared office-map rows sample code `0x04` and enter that region, then stop
on a separate neighboring `0x7F` record. L3 added two more live approaches to
Scene 2 record 1 and a four-direction lookup probe. The host matches the source
trace in all ten live routes; no gameplay collision change is indicated.

## Reproduction and source

Run Release CTest `shadowrun_ghidra_conversion_qt_field_movement_qa`. It
extends the ordinary field movement harness and saves full 320x224 frames and
every terrain probe under:

```text
build/conversion/qt/release/qt_runtime/field-movement-qa/object-bounds-census/
```

Each route contains `entry.bmp`, `contact-before.bmp`, `contact-after.bmp`,
`actual.bmp`, and `terrain-probes.tsv`. The parent `contacts.tsv` records the
ROM rows, source lookup row, input, fixed-point route, sampled corners, and
stop result. The full screen is the submitted artifact; the optional
`contact-center-zoom.png` files are inspection crops, not a visual baseline.

The primary data source is the US ROM file
`decompilation/input/shadowrun_genesis_us.bin`. Direct ten-byte reads at the
listed addresses match the active extracted scene records. The movement path
is the ROM path through `0x0BABC` / `0x0B77E`, collision classification at
`0x0E7C8`, low-code bounds lookup at `0x0E9EE`, and inclusive terrain raster at
`0x0EE10`. The source raster advances in 32-unit cells. Ghidra disassembly is
only a cross-check for those call sites and the `FFDDEC` record lookup.

| Scene | ROM record | ROM bytes | Rectangle and value | Result |
|---:|---:|---|---|---|
| 2 | `0x172DB4` | `02 50 02 5F 04 E0 05 1F 00 01` | `(592..607,1248..1311)`, `0x01` | Blocked by lookup record 1 |
| 2 | `0x172DBE` | `05 60 05 7F 03 40 03 4F 00 02` | `(1376..1407,832..847)`, `0x02` | Blocked by lookup record 2 |
| 2 | `0x172DC8` | `03 A0 03 BF 03 A0 03 AF 00 03` | `(928..959,928..943)`, `0x03` | Blocked by lookup record 3 |
| 7 | `0x1B3D90` | `03 80 03 9F 04 70 04 7F 00 01` | `(896..927,1136..1151)`, `0x01` | Blocked by lookup record 1 |
| 7 | `0x1B3D9A` | `05 00 05 1F 05 D0 05 DF 00 02` | `(1280..1311,1488..1503)`, `0x02` | Blocked by lookup record 2 |
| 8 | `0x1B463E` | `05 00 05 1F 04 10 04 1F 00 01` | `(1280..1311,1040..1055)`, `0x01` | Blocked by lookup record 1 |
| 31 and 34 | `0x1DB9F4` | `00 C0 00 DF 01 08 01 27 00 04` | `(192..223,264..295)`, `0x04` | Lookup is clear; neighboring record 20 blocks |

Scenes 31 and 34 share collision-list base `0x1DB5F8`. Their record 102 has
value `0x04`, so `0x0E9EE` indexes record 4 at `0x1DB620`, whose bounds are
`(640..671,1120..1247)` and value `0x7F`. That lookup does not contain the
runner at the workstation. The adjacent terrain boundary is record 20 at
`0x1DB6C0`, bytes `00 E0 00 FF 01 00 01 5F 00 7F`, rectangle
`(224..255,256..351)`. The host accepts the `0x04` candidate, enters the
record-102 area, and then rejects the neighboring `0x7F` candidate. This is the
ROM's current map data and lookup behavior; the bounds were not moved to fit
the artwork.

## Measured contact evidence

The deterministic route seed is `0x4000`, the field world counter is
`0x0200`, and every row asserts the exact pre-step position, candidate,
post-step position, four sampled bytes, and `0x7F` result. For the Scene
31/34 workstation, it also asserts the first accepted `0x04` candidate and
four-corner sample before the adjacent `0x7F` stop.

| Route | Before -> candidate -> after (fixed point) | Sampled corners | Outcome |
|---|---|---|---|
| Scene 2 record 1 | `(9258,20608) -> (9288,20608) -> (9258,20608)` | `00,01,00,01` | `0x01` rejected |
| Scene 2 record 2 | `(22742,13448) -> (22712,13448) -> (22742,13448)` | `02,00,02,00` | `0x02` rejected |
| Scene 2 record 3 | `(15104,15240) -> (15104,15210) -> (15104,15240)` | `03,03,03,03` | `0x03` rejected |
| Scene 7 record 1 | `(14122,18282) -> (14152,18282) -> (14122,18282)` | `00,01,00,01` | `0x01` rejected |
| Scene 7 record 2 | `(20266,23914) -> (20296,23914) -> (20266,23914)` | `00,02,00,02` | `0x02` rejected |
| Scene 8 record 1 | `(20266,16746) -> (20296,16746) -> (20266,16746)` | `00,01,00,01` | `0x01` rejected |
| Scene 31/34 record 102 | `(3368,4330) -> (3398,4330) -> (3368,4330)` | `04,7F,04,7F` | `0x04` lookup accepts; `0x7F` blocks |

For Scene 31/34, the first accepted lookup candidate is `(2888,4330)` with
corner bytes `00,04,00,04` and result `0x0000`. This enters the record-102
area. The later rejected candidate samples the neighboring record-20 boundary
and returns `0x007F`. The route does not claim that code `0x04` blocks the
workstation.

The first Scene 2 record-3 probe attempt approached from the west at `x=864`,
which is already inside record 6's `0x7F` rectangle `(864..895,928..1343)`.
That run never sampled record 3 and is excluded. The route now approaches from
the south at `x=944`, outside record 6, and the test pins the first actual
record-3 candidate. The first Scene 31/34 test also incorrectly required the
runner to leave the entire code-4 rectangle; the adjacent record-20 wall makes
that impossible. It now verifies the accepted lookup and the actual adjacent
source stop as separate observations.

## L3 four-corner and cardinal-direction extension

The Release and Debug movement QA now adds live east and north approaches to
Scene 2 record 1 at `0x172DB4`, complementing its existing west approach. Each
pins the attempted candidate, the committed position after the ROM fallback,
all four corner bytes, and result `0x7F`:

| Side | Candidate -> committed (fixed point) | Corner bytes |
|---|---|---|
| West | `(9288,20608) -> (9258,20608)` | `00,01,00,01` |
| East | `(9912,20608) -> (9942,20608)` | `01,00,01,00` |
| North | `(9600,19884) -> (9630,19824)` | `00,00,01,00` |

The fourth, south-side object sample is blocked by the scene's exclusive
terrain bound at normalized Y `1304`: the candidate `(9600,21088)` normalizes
to Y `1318`, so ordinary movement returns at the bounds check before object
lookup. To exercise the lookup handler itself, the test separately calls the
typed source-derived collision path with a widened *test-only* bounds record.
That isolated probe samples `01,00,00,00`, returns `0x7F`, and retains
`(9600,21128)`. Its TSV row explicitly says `inside_scene_bounds=0`; this is
not a playable south approach and does not change the map's bounds.

The same Release CTest now records Scene 19 record 8 at `0x1D5F28` from all
four cardinal input directions. Every candidate samples the ROM's `0x7E`
table rectangle and returns `0x7F`; the position after `0xBB86` fallback is
pinned separately from the rejected candidate. Scene 26 record 70 at
`0x1D9C2A` supplies the furniture-edge case: a Down+Right input reaches
candidate `(1420,3500)`, whose corner bytes are `00,00,00,7E`. The diagonal
candidate is rejected, the ROM-style axis fallback commits `(1420,3470)`,
and 30 subsequent neutral updates keep that position stable. The full capture
also verifies the field HUD and priority-drawn table remain present; no audio
request is added.

Scene 2 full-screen entry, contact-before, contact-after, and settled frames
are under
`build/conversion/qt/release/qt_runtime/field-movement-qa/object-bounds-census/scene-02-record-1-east/`
and `.../scene-02-record-1-north/`. Scene 26's full-screen sequence and all
30 neutral frames are under
`build/conversion/qt/release/qt_runtime/field-scene26-event1-entry-qa/table-collision/diagonal-corner/`.
The exact Scene 19 cardinal frames are under
`build/conversion/qt/release/qt_runtime/field-movement-qa/scene19-table-contact/`.
All are 320x224 host captures. There is no exact longplay contact frame for
these inputs. The reviewed frames show stable HUD/panel composition, the
runner remains visible, the table body is priority drawn over the actor while
its black shadow remains base-map art, and no new control or graphic overlaps
the HUD. Runner animation changes during the neutral tail while world
position stays fixed; this is the active host animation cadence and is not
claimed as an independently measured Genesis cadence.

## Visual review

All eight full 320x224 entry/contact/settled frames were opened at original
size. The HUD remains present, the runner is visible in the field, and the
contact state is not inferred from a crop. The zoom crops were used to inspect
feet, props, and terrain edges. For each route, `actual.bmp` is byte-identical
to `contact-after.bmp` after input release.

1. **Does it look correct?** Scene 2 shows the runner at the striped barrier
   and grayscale floor fixtures; Scenes 7/8 show the path and bridge edge;
   Scenes 31/34 show the workstation and adjacent divider. No new object or
   scenery was drawn.
2. **Does the look reflect behavior?** For the six matching records, the
   rejected corner contains the record's code and the runner remains on the
   approach side. For Scenes 31/34, code `0x04` is accepted as the ROM lookup
   says; the runner stops at the separate `0x7F` boundary.
3. **Are the visuals correct?** The capture uses the active extracted map art,
   priority layer, runner sprite, and HUD. The rectangles are checked against
   direct ROM bytes. This is not an independent Genesis pixel comparison.
4. **Should it animate?** These candidate props and path structures are static
   map art. Runner walk frames advance during held input; their source cadence
   remains the ordinary field animation path.
5. **Is there a sound?** None of these collision approaches emitted an audio
   request. The QA asserts an empty route audio-event list.
6. **Were nearby screen elements reviewed?** Yes: runner feet, candidate
   prop/edge, floor, priority overlap, HUD portrait, inventory slots, and ammo
   labels were reviewed on the full frames.
7. **Do multiple frames support the reading?** Each route has consecutive
   entry, contact-before, and contact-after frames. No longplay frame shows
   these exact inputs or contacts; no Genesis collision timing is inferred
   from the host frames.
8. **Were new controls drawn over old UI?** No controls or menu elements are
   added by this field route. The existing HUD remains visible and unmodified.
9. **Was the full time window reviewed?** The QA records every terrain probe
   through the first source stop, up to 120 field updates. It does not treat
   30-fps longplay samples as ROM frame timing.

The QA uses a dump-only camera setter to center the inspected world position
in each screenshot. It does not change movement, collision, or ordinary game
camera state. The independent longplay has no exact contact capture for these
rows, so this is ROM collision evidence plus a host presentation review, not a
claim of complete visual fidelity.

## Test result and disposition

Release CTest passes with exact position and corner assertions. The initial
west-approach and full-code-4-crossing attempts failed because their expected
routes ignored neighboring ROM records; those setup expectations were
corrected from the primary collision table. Debug and Release CTest pass, and
the eight settled-frame hashes match across builds. That hash check catches
build skew only. No host movement behavior or ROM collision data changed. The
census found no host stop that disagrees with the tested ROM rectangle/lookup
path. Scene 31/34's unusual code-4-to-record-4 mapping is documented above and
remains a source-data quirk, not a reason to invent a new rectangle.
