# OPEN-10: Ambush explosion patch has an opaque square

## Current reproduction

The Release full-intro capture at
`ghidra_conversion/build-ghidra-conversion-qt-release/qt_runtime/full-intro-ambush-fix-review/frame-004446.png`
is a complete 320x224 frame at wilderness `opening_frame=1012`. It shows a
rectangular tone boundary behind the center attacker explosion. The independent
Genesis reference is section 02 frame `000642.png`; the same region has no
patch-sized rectangle there. The current intro extractor's assets include four
152x128 `intro/background-patch-*.bmp` resources.

## ROM and extraction trace

The US ROM's ambush map pointer table begins at `0x5A1D2`. Its four entries
point to `0x1E2EBA`, `0x1E2F04`, `0x1E2F6C`, and `0x1E2EBA`, with encoded
lengths `0x0C`, `0x18`, `0x3C`, and `0x0C`. The source setup at
`0x5B4A8` selects the wilderness resources. These are ordinary source map
patches; the runtime is expected to composite their mapped pixels over the
existing scene, preserving empty cells.

The offline `decode_genesis_intro_ambush_background_resources_impl` decodes
each 13x12 map into a transparent 320x224 canvas. It currently calls the
0x1728 tile-map blitter with `transparent_zero=false`. In that mode zero-valued
cells and color-zero pixels become opaque, filling the map-patch extent. The
zero-cell policy must be checked against the ROM map and the patch's intended
layer before changing the extractor. This is an offline import correction;
the native renderer should consume the resulting named RGBA assets directly.

## Nine-question visual review

1. **Does it look correct? No.** The current full-screen frame has a visible
   rectangular boundary behind the center explosion; the independent frame
   `000642` does not.
2. **Does the look match behavior? Not yet.** The patch is sourced from the
   ROM's ambush map table, but the mapped zero cells currently overwrite the
   scene rather than remaining transparent. Confirm against the raw map and its
   drawing order before finalizing the change.
3. **Are the visuals correct? No.** The patch's outer canvas should not appear
   as a filled rectangle. Check all four patch assets and all surrounding
   actors, projectiles, text, and smoke after correcting transparency.
4. **Should it animate?** The map patch itself is static. The explosion/smoke
   sprites animate on their own source cadence; changing map transparency must
   not alter those frames or their timing.
5. **Does it have sound?** The patch renderer has no sound request. Audit the
   adjacent source event separately; do not infer a sound or duration from the
   pixels.
6. **Are other screen regions correct? Not established by this reproduction.**
   Review each full-screen checkpoint before and after the patch transition,
   including both attackers, the runner, text, shots, spell effects, smoke,
   palette fade, and later intro replacement.
7. **Do multiple frames support the interpretation? Yes, provisionally.** The
   independent frame `000642` and Qt frame `frame-004446` provide a
   cross-source comparison. Review multiple adjacent Genesis frames and the
   consecutive native intro run after the fix.
8. **Are new graphics layered cleanly? No.** The opaque patch is currently
   drawn over the prior background. Its empty cells must preserve the prior
   layer; verify no stale rectangle or newly covered actor remains.
9. **Was the full time window reviewed? Not yet for this correction.** The
   earlier full-intro capture identified the defect. A fresh consecutive
   Release capture and Debug/Release whole-screen checkpoints are required
   after the asset fix, with all changed frames reviewed.

## Status

**Reproduced; fix in progress.** Do not close OPEN-10 until the ROM map's empty
cells are traced, named assets are regenerated, the intro route passes Debug
and Release QA, and a fresh consecutive full-screen capture is reviewed.
