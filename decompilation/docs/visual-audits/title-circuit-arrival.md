# Title circuit arrival order audit

Date: 2026-10-06  
Scope: `BOOT-04`, title circuit visibility after the Shadowrun logo reveal.
Circuit color and texture remain separately tracked as `TITLE-03`.

## ROM trace

The US ROM is the primary source; the Ghidra export was used as a cross-check.

- Title setup at `0x1D7A` prepares the title resources and initial palette
  transition. The circuit is not made visible by that setup.
- Logo routine `0x1E4E` reveals the logo in columns. When the count reaches
  `0x25` (38 columns), it copies the circuit's target palette from ROM `0x2070`
  to RAM `FFD93E` and sets `DD5A=1`.
- Title text routine `0x1C1E` then starts countdown `0x5A`. It decrements on
  each title-loop call and requests a circuit palette step at countdown values
  `0x40`, `0x20`, and `0x00`. The copyright action follows after the third
  step.
- Palette routine `0x10D6` moves each CRAM component one source level toward
  its target. The circuit can therefore remain black after the logo has
  finished rolling across, then arrive through three distinct palette passes.
  The title background scroll continues behind it.
- The title loop's copyright sound request is separate from circuit arrival.
  This path has no circuit-specific sound request. The earlier SEGA shock
  sound is covered under `BOOT-01` and has been resolved separately.

## Native state and capture

`TitleTextState::palette_steps` counts the three source palette actions. The
Qt title presenter uses that typed count to select the corresponding cached
RGBA circuit palette. At zero passes, the circuit remains black while the
logo and scrolling state continue. It renders the first palette pass on the
same title-loop update that reaches the source countdown value `0x40`.

The startup QA saves whole `320x224` frames at title frames 188 and 189 and
records reveal completion, countdown, palette steps, and corner ink pixels:

| Title frame | Reveal complete | Countdown | Palette steps | Circuit corner ink |
|---:|---:|---:|---:|---:|
| 188 | yes | 65 | 0 | 0 |
| 189 | yes | 64 | 1 | 681 |

Release artifacts are `title-circuit-logo-complete-before-palette.bmp` and
`title-circuit-first-palette-step.bmp` under
`build/conversion/qt/release/qt_runtime/startup-qa/`. Both complete frames
were inspected. The black-to-first-pass order is correct; the first native
pass appears brighter than the compressed independent recording. That color
and texture discrepancy remains open as `TITLE-03`.

## Nine-question visual review

1. **Does it look correct?** The circuit now stays black through the full
   38-column logo reveal, then begins to appear after the logo has rolled
   across. Native whole-screen frames 188 and 189 show that order. Independent
   section 01 frames 000384 and 000390 show the corresponding logo-only and
   circuit-visible states. The native circuit's brightness remains a separate
   open discrepancy.
2. **Does the visible look match behavioral state?** Yes for arrival order.
   `DD5A`, the countdown, and typed `palette_steps` agree at the boundary:
   complete reveal with zero passes, followed by one pass at countdown 64.
3. **Are the visuals themselves correct?** The logo and circuit order match
   the source. The source-derived palette transitions are applied to the
   extracted circuit image. Exact brightness and thin-line texture are not
   confirmed by the compressed reference and remain `TITLE-03`.
4. **Should it animate?** Yes. The circuit background scrolls while the title
   loop runs. Its arrival uses three discrete palette steps at countdowns 64,
   32, and 0; it does not blink or loop through a visibility animation.
5. **Is there sound?** No circuit sound call is traced. The separate
   copyright sound request is not evidence of a circuit effect. The earlier
   logo shock sound is handled and verified under `BOOT-01`.
6. **Are the other screen parts correct?** The complete frames were reviewed:
   the rolled logo remains visible, the black interval contains no premature
   circuit detail, and no stale buttons or prior-screen graphics cover the
   title. Push Start, copyright, and `TITLE-03` keep their own checks.
7. **Do multiple frames support this interpretation?** Yes. Native frames 188
   and 189 straddle the palette boundary; independent section 01 frames 000384
   and 000390 support the same broad order. The 30-fps reference does not
   establish exact VBlank timing.
8. **Are new controls drawn over old UI?** No menu control is introduced by
   this transition. Each saved image is a complete title frame; no old screen
   buttons or panel graphics remain.
9. **Was the whole time window reviewed?** Yes for the current release
   verification. The full-intro capture covers the pre-title sequence, title
   reveal and menu, archetype selection, the complete Gator Shaman opening,
   field handoff, and a stable field hold. The continuous capture and boundary
   log are retained under
   `build/visual-audits/intro-full/release-gator-shaman-20261006/`. All frames
   are whole-screen; separate boundary frames were inspected at native size.

## Verification and disposition

- **Code changed:** the title's typed text state now counts palette passes;
  the Qt presenter keeps the circuit black until the ROM palette transition
  begins and applies each source pass thereafter.
- **Behavior implemented:** 38-column reveal completion, initial black
  interval, palette steps at countdown 64/32/0, and continued title scrolling.
- **Internally checked:** `shadowrun_ghidra_conversion_qt_startup_qa` checks
  full-screen pixels and typed state at frames 188/189. The title-loop
  conversion test checks the three palette actions before copyright.
- **Independently visually checked:** complete reference frames 000384 and
  000390 show logo-only then circuit-visible ordering. Their coarse 30-fps
  timing is not treated as a VBlank oracle.
- **Reported defect resolved:** yes for early circuit appearance. Circuit
  color/texture and the separate SEGA shock sound remain open.
