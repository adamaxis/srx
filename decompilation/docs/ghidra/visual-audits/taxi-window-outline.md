# Taxi-window outline audit

**Status:** `TAXI-01` is resolved for the missing top and bottom edges. The
cab-map plate's position and extent also match the sampled independent frames.
Exact video-to-VBlank phase and the controller event that selected each
destination remain unverified.

## Source path and cause

The US ROM path enters `run_post_intro_mode` at `0x134D2`. Its alternate
branch calls `render_mode_selection` at `0x13726` for mode 4, holds that mode,
then calls the same routine for mode 2 before handing off to the field.
`0x13726` indexes the selected cursor record through `0xD445D` and `0xD4464`,
reads the eight-tile border at `0xD4446`, and places it from `0xD4536`. Each
of its five size records is presented across two VBlanks.

The runtime already had the extracted tiles, but `draw_cab_mode_selection()`
looked up `top-left`, `top-middle`, `top-right`, and corresponding bottom
aliases. `resources.tsv` names those assets with dots, such as
`intro.cab.cursor.top.left`. Thus the six multi-part aliases did not load;
only `left` and `right` drew. Debug startup QA reproduced the defect at
`opening_transition_3048_post_intro_cab_map.bmp` and failed on the missing
top-left tile. The renderer now requests the catalog's dot-separated names.

The runtime continues to consume the eight named extracted assets. No border
geometry or replacement artwork was invented.

## Nine-question review

1. **Does it look correct? — Yes, for the reported missing edges.** The
   independent section 02 full-screen frames 001729-001735 show the callout
   closing and shrinking to Downtown Seattle; frame 001800 shows the Redmond
   selection. The fixed Qt images have a closed outline through both mode
   sequences. Registration of the extracted map plate against eight
   independent frames places it at the same `(32,32)` origin as Qt; details
   are recorded below.
2. **Does the appearance match the behavioral state? — Yes.** The typed
   `post_intro_mode`, `post_intro_phase`, and
   `post_intro_caption_visible` select the current record and caption. QA
   checks all eight asset tiles at the ROM-derived Plane-A coordinates in all
   five phases and in the settled mode-4 and mode-2 states.
3. **Are the visuals correct? — Yes for the border tiles and their placement.**
   The renderer now resolves all eight catalog aliases and draws the source
   images. The full-screen pixel checks fail if any tile is missing or does
   not match its source image at the planned position. Independent-frame
   registration also supports the extracted plate at its ROM-derived position.
4. **Should it animate? — Yes.** ROM `0x13726` presents five outline sizes at
   two VBlanks each. ROM `0x134D2` holds mode 4 for its `0x40` countdown and
   mode 2 for its `0x60` countdown, then returns to the field path. The full
   Release capture contains 182 consecutive cab-map frames, opening frames
   3048-3229. Independent 30-fps frames support the sequence but cannot fix
   its exact VBlank phase.
5. **Is there a sound effect? — No cursor/caption cue was found.** The traced
   `0x134D2`/`0x13726` path has no sound request tied to the outline or
   caption; no separate clip or duration is assigned.
6. **Are related screen parts correct? — Reviewed.** The full-screen pass
   includes the preceding airport text/background, map, captions, selector,
   and first field screen. The extracted plate registers at its ROM-derived
   position and full extent in the independent frames.
7. **Do multiple frames support the interpretation? — Yes for the outline;
   input cause is unknown.** Independent frames 001729-001735 and 001800 show
   the callout at several sizes and destinations; frame 001810 shows the
   handoff black frame. The recording has no controller log, so it does not
   identify the exact input associated with a selection.
8. **Does the new UI replace the old UI cleanly? — Yes in the captured route.**
   Airport graphics and text are gone on cab-map entry. Downtown is replaced
   by Redmond without stale caption text, and the first field image has no
   cab-map outline or caption.
9. **Was the whole time window reviewed? — Yes for this affected route.**
   The Release full-intro capture has 4,982 full-screen frames. I reviewed
   every frame in the affected window: 182 cab-map frames followed by 120
   field-handoff hold frames, from capture frames 4680-4981. The prior 198
   frame Debug/Release range dumps also match byte for byte. The separate
   field-handoff frames verify that the taxi UI is cleared on entry.

## Verification evidence

- `shadowrun_ghidra_conversion_qt_startup_qa` passed in Debug and Release.
- Startup QA now saves 12 named full-screen checkpoints: each of five phases
  and a settled hold for mode 4 and mode 2. Its eight-tile pixel assertion
  reproduces the old bug before the alias fix and passes after it.
- Debug and Release startup QA each produced 130 BMPs with matching hashes.
  Compared with the preserved pre-fix Debug run, 12 are new checkpoints and
  the existing cab-map transition image is the only changed prior image.
- The Debug/Release sequential opening dumps each contain 198 full-screen
  frames for frames 3030-3227; every image is 320x224 and the two sets match
  byte for byte.
- Release CLI `--startup-dump opening 2 3133` writes
  `archive/qa-artifacts/visual-review/taxi-outline-cli/actual.bmp`. It is 320x224 and matches
  `cab_cursor_mode2_settled.bmp` byte for byte.
- The full Release capture and reviewed contact sheets are under
  `archive/qa-artifacts/visual-review/taxi-full-intro-release/` and
  `archive/qa-artifacts/visual-review/taxi-full-intro-contact/`.

**Code changed:** Qt cursor asset lookups now match the catalog.
**Behavior implemented:** all eight border tiles render for each typed
selection size and destination.
**Independently visually checked:** section 02 frames 001729-001735,
001800, and 001810 support the closed outline, selected map states, and
handoff classification. The plate registration results below support its
position and extent.
**Reported defect resolved:** the missing top/bottom edges are fixed, and the
reported plate-extent difference is not reproduced. Exact video-to-VBlank
alignment and controller input remain open.

## Cab-map plate registration follow-up

The US-ROM path at `0x134D2` loads the map stream from `0xD2A2E` into the
Plane-B name table beginning at VRAM address `0x2208`. With the plane base at
`0x2000` and 64 tile cells per row, that is row 4, column 4: screen position
`(32,32)` at zero scroll. The offline graphics extractor writes that source
resource as `assets/intro/cab-map.bmp`; its dimensions are 208x136 pixels
(26x17 tiles). The Qt presenter draws the full asset at the same source-derived
position.

I registered the grayscale plate asset against each independent 320x224
section-02 screenshot using full-frame normalized cross-correlation. The
maximum location is `(32,32)` in all eight sampled frames:

| Independent frame | Peak score |
| --- | ---: |
| `001729` | 0.99136 |
| `001730` | 0.98627 |
| `001731` | 0.98557 |
| `001732` | 0.99004 |
| `001733` | 0.99214 |
| `001734` | 0.96411 |
| `001735` | 0.96412 |
| `001800` | 0.96557 |

The later frames include cursor/caption overlays, and all reference frames are
compressed video captures. These scores support the plate's origin and full
extent; they do not establish pixel identity or VBlank phase. The old extent
discrepancy came from visual comparison and is superseded by this registration
and the ROM placement trace.
