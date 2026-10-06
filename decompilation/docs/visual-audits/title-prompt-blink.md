# Title Push Start Blink Audit

Date: 2026-10-06
Scope: `TITLE-01` only. The Start-to-options transition remains a separate
open goal (`TITLE-02`).

## Source trace

Primary source is the US ROM title loop at `0x1A9E` and its title setup at
`0x1D7A`. Setup initializes D7 to `0x0E10`. Each loop waits for VBlank, updates
the other title state, then checks D7 against `0x0C6C`. The loop draws a prompt
record only when D7 is below that threshold. It chooses variant B when
`D7 & 0x3F < 10`, otherwise variant A, then decrements D7 for the next loop.
The first prompt request is therefore title frame 422: each of the preceding
421 loop iterations tested D7 at or above the threshold, leaving D7 at 3179
for the first prompt request.

`0x1DCA` sends record `0x1F81` to `0xCA84` with palette word `0x2000`; its text
is `PRESS START`. `0x1DE0` sends record `0x1F99` to the same destination and
palette; that record is blank. Both call the common text writer at `0xC0DA`.
Ghidra's export was used to cross-check the disassembly and direct callees.

The typed scheduler in `title_loop_conversion.cpp` already selected these
records at the correct counter values. The Qt presenter had set
`title_prompt_visible_` to true for every render request, including record B.
It now derives visibility from the selected `TitleTextAsset`; the clean
per-frame title composition omits the top seven text-image rows while B is
active and retains the separate copyright rows.

## Eight-question visual review

1. **Does it look correct? — Yes for the reported blink.** Independent 320x224
   section 01 frames 000900, 000906, and 000912 show the label visible, absent,
   and visible again. The exact relationship between video frame numbers and
   title D7 is not logged, so these frames verify the visual sequence rather
   than absolute phase.
2. **Does the visible look match the behavioral state? — Yes.** No selection
   cursor applies to this prompt; visibility is its state marker. Variant A
   means visible and variant B means blank in the ROM records. The Qt state
   and screenshots now follow the selected typed variant. QA records D7
   remaining values and verifies the expected variant and visibility on each
   checkpoint.
3. **Are the visuals themselves correct? — Yes for the prompt.** The extracted
   `title.text` asset is drawn at its source catalog position. All 233
   foreground prompt pixels match the asset in visible frames and none match
   during blank frames. All 2,271 copyright foreground pixels match in every
   checkpoint.
4. **Should anything animate? — Yes.** D7 counts down once per VBlank. The
   first visible run is 34 frames because the counter enters the cycle at low
   bits 43; it is followed by 10 blank frames. Later cycles contain 54 visible
   and 10 blank frames, totaling 64. The loop does not pause. It exits on a
   latched controller event or when D7 underflows.
5. **Does it have sound? — No call traced.** The title loop and both prompt
   draw routines contain no sound request. There is no blink-specific effect
   whose playback duration needs measuring.
6. **Are the other visible parts of the screen also correct? — No, not yet.**
   All five reviewed native checkpoints are complete 320x224 frames. The logo,
   copyright, and prompt region remain stable as expected, but the title
   circuit background is much brighter and denser green than the independent
   frames. That separate full-screen finding remains open as `TITLE-03`.
7. **Do multiple frames support the interpretation? — Yes.** Native frames
   cover title frames 422, 456, 466, 520, and 530: visible, blank, visible,
   blank, visible. Independent frames 000900/000906/000912 show the same
   visible/blank/visible sequence. Their 30-fps spacing is too coarse to prove
   the 60-Hz counter cadence; the ROM trace and typed regression cover that.
8. **Does the new UI replace the old UI cleanly? — Yes.** This is one title
   layer, not a child page. Variant B submits a blank text record and the host
   rebuilds each full frame, leaving no prompt glyphs behind. Copyright rows
   and the rest of the title remain drawn. No stale controls or graphics are
   layered under a replacement UI.

## Verification

- Typed CTest: `shadowrun_ghidra_conversion`, Debug and Release. It checks the
  421-update delay, the initial 34-visible/10-blank transition, two 64-frame
  cycles, and terminal scheduler behavior.
- Full-screen Qt CTest: `shadowrun_ghidra_conversion_qt_startup_qa`, Debug and
  Release. It writes `title-prompt-visible-first.bmp`,
  `title-prompt-hidden-first.bmp`, `title-prompt-visible-second.bmp`,
  `title-prompt-hidden-second.bmp`, and `title-prompt-visible-third.bmp`, plus
  `title-prompt-qa.tsv`. Every image is 320x224; all five Debug/Release pairs
  are byte-identical. Every Debug full-screen image was opened and reviewed.
- **Code changed:** the Qt title presenter now respects the selected
  `TitleTextAsset`; startup QA records two prompt cycles and checks prompt and
  copyright pixels.
- **Behavior implemented:** the visible and blank phases follow the ROM
  countdown and low-six-bit test.
- **Internally checked:** the typed schedule and rendered pixels pass in both
  build configurations.
- **Independently visually checked:** section 01 frames 000900, 000906, and
  000912 support visible/hidden/visible behavior.
- **Reported defect resolved:** the Push Start label now blinks as reported.
  The unrelated title background color/texture and Start transition remain
  open under their own goals.
