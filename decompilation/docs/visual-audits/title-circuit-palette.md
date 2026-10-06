# Title Circuit Color and Texture Audit

Date: 2026-10-06
Scope: `TITLE-03`. Circuit motion is tracked separately under `BOOT-03`;
Push Start and title transition are tracked under `TITLE-01` and `TITLE-02`.

## Finding

The native title's circuit is visibly brighter and denser green than the
independent gameplay capture, including after aligning the frames by a visible
screen event and by background geometry. The visual mismatch is real in the
available images, but the provided reference is compressed video and does not
reliably preserve thin green lines. ROM evidence supports the current extracted
base palette and does not reveal a global circuit-darkening step. No palette
correction is justified yet; `TITLE-03` remains unresolved.

This audit also corrected a QA labeling error. Startup QA used to advance
through the prompt checkpoints to title frame 530 before saving the image
labeled `title_300.bmp` and copying it to `actual.bmp`. That image was not
frame 300 and could not support the earlier frame-300 comparison. The capture
order is fixed: the title reference checkpoints are saved first, and the
frame number, scroll offset, and light phase are written to
`title-scroll-qa.tsv`. `title_300.bmp` and `actual.bmp` now represent title
frame 300.

## ROM trace

Primary evidence is the US ROM; the Ghidra export was used as a cross-check.

- Startup display setup at `0x0CEC` loads the VDP register table. The register
  12 setup at `0x8C81` leaves shadow/highlight disabled; that feature cannot
  explain a globally dimmed or brightened circuit.
- The circuit map at `0x67DF6` selects palette line 2. The source palette
  words are in the 64-word table at `0x1C024`. The palette values include
  `0x0020`, `0x0040`, `0x0060`, `0x0080`, `0x00A0`, and `0x00E4`; the
  extractor expands Genesis channel levels to RGBA8. This supports the
  currently extracted line colors.
- Title light routine `0x2634` runs only while title text state `DD5A==2`.
  Its countdown advances through seven phase values and writes eight words
  beginning at CRAM `$50` from `0x26F2 + (DF92 << 1)`, at five-count
  intervals. It modifies the moving light phase, not the base green entries
  used by the circuit.
- Timer routine `0x26D0` updates the background VSRAM offset on the first
  title-loop frame and then once per five VBlanks. The typed native title
  offset follows this cadence. The scroll is a title-loop animation and
  remains active while that loop runs.
- Title setup performs its 17-pass palette-target fade before the main title
  loop. No later global circuit fade was found. Routines `0x26D0` and
  `0x2634` contain no sound call. A separate sound request occurs at the
  copyright draw path in `0x1C1E`; it is not tied to circuit motion or the
  light phase, and its output duration is not measured here.

## Matched whole-screen review

Independent section 01 video frame 000418 is the first sampled frame with the
copyright text; 000416 and 000417 have no copyright. The ROM title loop draws
the copyright at title frame 254. This observed event anchors candidate
comparisons; it does not assert that the video's frame zero equals a ROM
VBlank. The neighboring circuit geometry confirms the candidate vertical
alignment. Its best shift is zero pixels in the sampled pairs except frame
000424, whose compressed geometry favors a one-pixel shift.

| Independent section 01 frame | Native title frame | Native scroll | Light phase |
|---:|---:|---:|---:|
| 000390 | 198 | 40 | 0 |
| 000396 | 210 | 42 | 0 |
| 000418 | 254 | 51 | 1 |
| 000420 | 258 | 52 | 1 |
| 000421 | 260 | 52 | 2 |
| 000422 | 262 | 53 | 2 |
| 000423 | 264 | 53 | 3 |
| 000424 | 266 | 54 | 3 |
| 000425 | 268 | 54 | 3 |
| 000450 | 318 | 64 | 6 |

All 12 newly labeled native whole-screen captures were reviewed, including
the consecutive frames surrounding the copyright appearance and circuit
changes. The independent frames 000416–000425 were also reviewed at full
320x224 resolution, as were 000390, 000396, and 000450. In the matched pairs,
the logo and copyright placement agree, and the circuit geometry moves
upward. The Qt circuit retains substantially more luminous green detail than
the video. Compression and capture color processing can crush these narrow
lines, so the video is insufficient to determine a replacement RGBA value.
The top-band green-pixel count is not used as a color target.

## Eight-question visual review

1. **Does it look correct? — No.** The native circuit has more visible green
   detail and brightness than the independent frames after event and geometry
   alignment. Whole-screen references: section 01 frames 000390, 000396,
   000418, 000420–000425, and 000450. The independent video is compressed.
2. **Does the look match behavioral state? — Yes for motion and phase.** The
   QA trace records title frame, scroll offset, and phase. At frames 198–318,
   the offset rises from 40 to 64 and the light phase progresses from 0 to 6.
   The ROM advances the background offset every five VBlanks and uses the
   separate countdown for the seven light phases. This does not settle the
   color mismatch.
3. **Are the visuals themselves correct? — Unknown for thin-line color;
   geometry is supported.** The ROM map and palette table support the named
   asset mapping and base colors. Matched full frames support the relative
   circuit placement and upward movement. The compressed capture cannot
   establish the correct low-intensity green values. Do not tune color from
   that signal alone.
4. **Should it animate? — Yes.** Background scroll begins on the first title
   loop frame and advances once every five VBlanks while the title loop is
   active. The title light counter advances through seven phases on its
   five-count cadence while `DD5A==2`. The title loop ends on its controller
   transition or terminal countdown; no pause condition was found.
5. **Is there a sound? — No circuit sound call traced.** Neither the scroll
   routine nor the light-phase writer requests sound. The copyright path has
   a separate request; duration and physical playback are outside this
   circuit finding.
6. **Are other screen parts correct? — Reviewed; separate defects remain.**
   The full 320x224 images show the logo and copyright on the expected layers
   and no extra menu controls. The circuit mismatch remains. Push Start blink
   and Start transition keep their separate goal statuses.
7. **Do multiple frames support this reading? — Yes.** Twelve native title
   checkpoints record several offsets and phases. Independent frames 000416
   through 000425 show consecutive motion and the copyright boundary;
   frames 000390/000396 and 000450 provide earlier/later positions. The video
   runs at 30 fps and cannot establish exact 60-Hz VBlank phase by itself.
8. **Does new UI replace old UI cleanly? — Yes; no child page is present.**
   The title is rebuilt as a complete frame at each checkpoint. No stale
   buttons, graphics, prompt glyphs, or menu panel are drawn over the title.
   The copyright and logo remain visible through the tested background
   phases.

## Verification and disposition

- **Code changed:** startup QA now captures correctly numbered full-screen
  title checkpoints and asserts the copyright draw boundary in
  `decompilation/ghidra_conversion/qt_runtime/src/ghidra_conversion_qt_main.cpp`.
- **Behavior implemented:** the existing typed five-VBlank scroll and
  seven-phase title light schedule are now captured at correctly labeled
  title frames.
- **Internally checked:** `shadowrun_ghidra_conversion_qt_startup_qa` passed
  in Debug and Release. The 12 matched frame files are 320x224, and all
  Debug/Release pairs have matching SHA-256 hashes. `actual.bmp` and
  `title_300.bmp` both hash to
  `8E6C5207DDE56D7196F3762C5C826A0468761FA4106782CD4B779FFFE70C29CE`.
- **Independently visually checked:** section 01 frames 000390, 000396,
  000416–000425, and 000450 support the copyright anchor, changing circuit
  geometry, and upward movement. Their compressed color values do not settle
  thin-line brightness.
- **Reported defect resolved:** no. The visible brightness/texture mismatch
  remains open as `TITLE-03` pending stronger independent color evidence or
  another source-backed cause.
