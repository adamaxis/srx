# Boot to title black interval audit

Date: 2026-10-06  
Scope: `BOOT-05`, the black hold after FASA and before the first Shadowrun logo
column. The missing item was an inventory gap; this audit does not claim that
the current build omitted the screen.

## ROM trace

The US ROM bytes are primary. Disassembly labels below describe those bytes;
the Ghidra export was used only as a cross-check.

- Reset enters the pre-title routine at `0x5D746`. It runs its SEGA animation
  loop with one `0x0BD2` VBlank wait per iteration. Afterward it samples input,
  fades through `0x0131C`, calls `0x5D886` to replace the display resources,
  then waits 121 VBlanks (`D7=0x78`, inclusive DBF) for FASA before the final
  input sample and palette step. This is the end of the FASA screen.
- The caller then enters title loop `0x1A9E`, whose first action is setup
  `0x1D7A`. It initializes the title resources and calls `0x12F6`; it does not
  reveal a logo column there. The loop sets `DD2C=0x5A` at `0x1AC6`, waits once
  per title update at `0x1B02`, and decrements `DD2C` at `0x1B24`.
- While the post-decrement value is nonnegative, the branch at `0x1B2A`
  skips `0x1E4E`. Thus the screen remains in its initial black palette and
  shows no logo column for 90 title updates. On update 91, the value becomes
  `-1` and the loop calls `0x1E4E`, which emits the first eight-pixel column.
  `DD40` then spaces subsequent column writes by two title updates.
- The circuit stays black while the logo rolls. The completed reveal copies
  its target palette, then `0x1C1E` applies the three circuit palette steps;
  that later arrival order is covered separately by `BOOT-04`.
- No audio call is made during the 90-update black hold. `0x1E4E` has a
  separate command `0x5D` request on alternating logo-column calls. The
  earlier SEGA shock is handled under `BOOT-01`: `0x5C` starts resource 92
  during the lightning window, while `0x16` follows an `FF 12` stop-resource-
  22 operation. Neither request occurs during this title black interval.

## Native state and whole-window evidence

`TitleLoopState::logo_reveal_delay` starts at `0x5A`. Qt draws an opaque
320x224 scene for each host update. The boot FASA QA now checks the complete
image on both post-FASA handoff updates, the title handoff, and each of the
next 90 source updates. At every title-delay update, the delay value, zero
revealed columns, zero circuit palette steps, and hidden transient sprite
state must match the pixels: every pixel is black. The following update must
have delay `-1`, exactly one revealed column, and visible pixels. The QA writes
full-screen boundary images and
`title-black-interval.tsv` under `qt_runtime/boot-fasa-qa/`.

In the continuous Release run, frames 540 through 650 are fully black. Frame
651 contains the first visible Shadowrun mark column; the full logo continues
rolling in afterward. The 6,782-frame capture proceeds through the entire
intro, field handoff, and stable field hold. Its process exited normally. The
all-screen contact sheets and the full-sized transition frames were reviewed;
the black hold contains no FASA/SEGA remnants, circuit lines, stale controls,
or stray sprites.

Independent section 01 frames `000300` and `000330` show the black interval;
frame `000360` shows a partial Shadowrun logo. The uploader text was excluded
from game-pixel comparison. The 30-fps recording supports sequence order but
cannot measure the ROM's 60-Hz update count.

## Nine-question visual review

1. **Does it look correct? — Yes.** The complete native interval is black
   between the faded FASA screen and the first rolling Shadowrun mark.
   Independent section 01 frames `000300`, `000330`, and `000360` support the
   same black-then-logo order. The video cadence is too coarse for exact frame
   timing.
2. **Does the look match behavioral state? — Yes.** The 90-update delay,
   zero logo columns, zero circuit palette steps, and hidden transient sprite
   agree with the black pixels. The next update decrements the counter below
   zero and reveals one column.
3. **Are the visuals correct? — Yes for this interval.** The screen is a clean
   full-frame black; the previous FASA logo, SEGA graphics, title circuit, and
   title controls do not remain underneath. Later logo color/texture and
   circuit color remain separate title goals.
4. **Is it animated? — The hold is static for 90 title updates.** The title
   background offset timer still advances every five updates, but the circuit
   is black so that motion is not visible. The logo then reveals by one
   eight-pixel column every two updates; the source stops at 38 columns.
5. **Is there sound? — No call is traced during the black hold.** The separate
   command `0x5D` occurs on alternating logo-column calls after the hold;
   source sound identity and playback duration are outside this visual slice.
   The earlier SEGA shock event is covered separately under `BOOT-01`.
6. **Are the other visible parts correct? — Yes in the reviewed frames.** The
   complete image has no old logo, circuit, button, text, or stray sprite over
   the black interval.
7. **Do multiple frames support this interpretation? — Yes.** Qt checks both
   post-FASA handoff frames and every title update from frame 0 through 90,
   saving title frames 0, 1, 45, and 90 followed by the first-column frame 91.
   The independent recording supplies two black samples and one partial-logo
   sample.
8. **Does new UI replace old UI cleanly? — Yes.** No UI is introduced here.
   The title handoff redraws a full opaque frame; no FASA/SEGA art or controls
   remain.
9. **Was the whole time window reviewed? — Yes.** The continuous
   6,782-frame Release capture covers reset, every pre-title/title phase,
   Start Options, archetype selection, the full Gator Shaman intro, field
   handoff, and a stable field hold. It completed without a crash. This
   focused QA adds per-update black-pixel checks for the 90-frame interval.

## Verification and disposition

- **Code changed:** only the Qt boot QA and its output trace/checkpoint dumps;
  the renderer already presented the black interval.
- **Behavior implemented:** the existing typed title delay and first-column
  reveal match ROM `0x1A9E`/`0x1E4E`.
- **Internally checked:** `shadowrun_ghidra_conversion_qt_boot_fasa_qa`
  asserts the complete black frame across the FASA handoff and all 90 title
  delay updates, then checks the first visible column on update 91 in Debug
  and Release.
- **Independently visually checked:** section 01 frames `000300`, `000330`,
  and `000360`; these establish order, not exact VBlank timing.
- **Disposition:** the reported omission from the defect inventory is closed.
  The black-screen behavior is present in the current build and is not a
  reproduced rendering defect.
