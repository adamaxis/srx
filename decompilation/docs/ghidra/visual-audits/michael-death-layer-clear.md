# Michael death-pose layer clear audit

Scope: `OPEN-03`, the death image appearing over the earlier wilderness actor
plate. Visual scope is resolved. The source audio request at the same event is
reviewed separately in [the terminal audio audit](opening-terminal-audio.md);
it selects a verified silent slot, so `OPEN-07` is closed as a non-defect.

## ROM trace and implementation

The US ROM bytes at `0x59FBE` load destination `$0600`, width counter
`0x10`, and height counter `0x0F`, then call `0x5B32C`. That helper writes
blank tile word `$07FF` with row stride `$80`; its DBF loops make the fill
17 columns by 16 rows. The same terminal branch then sends the final death
image from `0x1E2E3E` to `$0786` as a 20-by-13 stream. The overlay service
enters this branch when its elapsed counter reaches `0x168` and does no more
overlay writes afterward. The stage-2 loop later exits to the tri-vid scene.
`export/disassembly.s` is the secondary cross-check for this
ROM trace.

The prior Qt path treated the blank fill as an opaque generic transition and
kept the left map or its last overlay in persistent presentation state. The
sixth overlay asset happens to match the initial left plate pixel for pixel,
so choosing one of those two images over the other could not clear the art.
The typed overlay callback now clears the left wilderness plate before the
final image request. The Qt controller resets both left-region layers; it
does not recreate the source tile map at runtime.

## Evidence window

- Full Release capture: `archive/qa-artifacts/visual-audits/intro-full/release-open03-clear-20261006/`.
  It contains 6,782 consecutive 320x224 frames from boot through a 120-frame
  stable field hold, the phase trace, and 18 contact sheets sampling the whole
  run every six frames. All 18 sheets were reviewed in order; full-size frames
  were inspected around the death transition and handoff.
- Death-layer QA captures full-screen frames 688, 752, 850, 1021, 1093,
  1208-1211, 1266, 1354, and 1366, plus the tri-vid entry at 1382. Its
  `actual.bmp` is the post-clear death pose at opening frame 1209. Debug and
  Release outputs are under
  `decompilation/ghidra_conversion/build/conversion/qt/{debug,release}/qt_runtime/opening-death-layer-qa/`.
- Independent section 02 source frames 730, 740, 750, 760, 770, and 780 were
  inspected as whole screens. Frame 730 shows the earlier pose; 740-760 show
  the death pose without the extra left-side figure fragment. Frame 770 has a
  legitimate projectile crossing the left edge. That source example corrected
  an over-broad first QA assertion that had required this edge to stay black
  for the whole death-pose lifetime.
- Native full-run frames 4642-4645 map to opening frames 1208-1211. The old
  pose is present at 1208; the terminal clear and death pose are visible at
  1209; 1210-1211 retain the clean pose. At later frames, the typed left-layer
  state remains cleared while legitimate projectiles continue to move.

## Nine-question review

1. **Does it look correct? — Yes for the reported overlap.** The source
   recording shows the death pose without the previous left-plate fragment;
   the corrected native transition does the same. The reference is a 30-fps
   gameplay recording, so it supports appearance and event order, not exact
   VBlank alignment.
2. **Does visible state match behavior? — Yes.** At the ROM terminal event,
   typed state removes the left map and overlay before the final-image request.
   QA checks that neither layer is present during every remaining death-pose
   frame and through the tri-vid replacement.
3. **Are the visuals correct? — Yes for this layer lifetime.** The blank
   region is replaced before the final 20-by-13 image. Full-screen output at
   1208-1211 shows the handoff, and `actual.bmp` is read back after saving.
4. **Should it animate? — The scene has timed motion around a static death
   pose.** The six left-region overlays use source delays 12, 12, 12, 24, 24,
   and 12 frames. At elapsed `0x168`, the left plate clears and the final pose
   is installed; later overlay calls are no-ops. Projectiles still move across
   the scene and are not part of the retired plate.
5. **Does it have sound? — The source request selects a silent slot.** ROM
   `0x59F8E` sends `0x2E` as `FF 10 2E`, which selects table-2 slot 46. The
   ROM-backed extractor produces only setup/quieting writes for slot 46; it
   has no extracted sound asset or playback duration. Leaving it unmapped in
   `AudioPlaybackService` is correct, and `OPEN-07` is closed as a non-defect.
   See the [terminal audio audit](opening-terminal-audio.md).
6. **Are the other visible parts correct? — Reviewed across the whole run.**
   The capture includes the title, menus, all story scenes, cockpit, airport,
   cab map, and field arrival. The known title-circuit brightness/density
   mismatch remains `TITLE-03`; no new stale UI or unrelated overlay was
   observed in this run.
7. **Do multiple frames support the interpretation? — Yes.** Native frames
   1208-1211 and 1266/1354/1366 were inspected with independent source frames
   730-780. The matching sequence distinguishes the earlier pose, the clear,
   the stable death pose, and later projectile motion.
8. **Does the new presentation replace the old one cleanly? — Yes.** The
   semantic clear callback resets both old left-region layers before recording
   the final pose. QA checks their absence until the whole wilderness scene is
   replaced by tri-vid.
9. **Was the entire time window reviewed? — Yes.** The Release capture runs
   from the opening black interval through all intro scenes and into the
   stable field hold; all 18 whole-screen contact sheets and the full-size
   death-transition checkpoints were reviewed. The process completed without
   stalling or crashing. This capture has no audio track.

## Verification result

- **Code changed:** `title_intro_overlay_conversion.cpp`,
  `opening_intro_conversion.cpp`, and the Qt presentation callback.
- **Behavior implemented:** the typed terminal event clears the stale left
  plate before drawing the death pose and preserves that clear until scene
  replacement.
- **Internally checked:**
  `shadowrun_ghidra_conversion_qt_title_intro_overlay_qa` verifies callback
  order; `shadowrun_ghidra_conversion_qt_opening_death_layer_qa` verifies the
  transition pixel state, full lifetime, and tri-vid handoff. Both pass in
  Debug and Release. The CLI artifact is `opening-death-layer-qa/actual.bmp`.
- **Independently visually checked:** gameplay section 02 frames 730-780 and
  the complete 6,782-frame source-build capture were reviewed. The external
  source supports the cleared death pose; 30-fps spacing does not establish
  exact VBlank phase.
- **Reported defect resolved:** `OPEN-03` is resolved for the stale actor
  plate showing under Michael's death pose. The separate `0x2E` request selects a verified silent slot;
  `OPEN-07` is closed as a non-defect.
