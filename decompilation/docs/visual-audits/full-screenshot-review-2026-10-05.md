# Full screenshot review — 2026-10-05

## Expanded whole-frame pass

The original version of this note called a selected set of directories the
complete screenshot corpus. That was wrong: it missed thousands of rendered
screenshots in ignored build trees. The old counts below describe only that
narrow set and must not be used as the whole-workspace inventory.

The expanded pass indexed 7,064 full-frame BMP/PNG screenshot files in the
build, runtime, and visual-audit output trees. Exact-pixel deduplication
grouped them into 2,298 distinct frames: 2,239 at 320x224, 58 at 514x360, and
one at 640x448. Every distinct 320x224 frame was reviewed in one of 144
whole-frame contact sheets. The larger images were reviewed at their original
resolution in 15 additional contact sheets. The index maps each of the 7,064
source paths to its reviewed image, so Debug/Release duplicates and repeated
captures remain traceable.

I excluded resource-only exports, individual extracted assets, and a 1280x896
shop mosaic whose individual full-frame captures were already included. The
five excluded asset/refresh export trees contain 5,280 image files, including
42 distinct 320x224 background, map, panel, and title-animation plates. I
reviewed those full-canvas plates separately; they are named source resources,
not rendered interface captures. The 21,609 numbered FFmpeg frames from the
independent longplay are source-video reference frames, not application
screenshot outputs; I did not review that
entire reference archive frame by frame. I inspected the relevant section 02
frames individually when checking the opening sequence. The index, contact
sheets, and native-size sheets are in
`%TEMP%\shadowrun-full-screen-review-20261005` on this machine; they are
review aids, not checked-in build outputs.

This supersedes the earlier selected-set count: the two Qt runtime trees had
152 Debug and 139 Release images in that narrow pass, with 139 matching paths
pixel-identical. The old 730-candidate inventory reduced to 437 unique
images. Those figures remain valid only for the specific directories that
were originally enumerated. The old report also sampled 18 clean section 01
reference frames; that was never a full review of the longplay source.

## Scope and method

I reviewed whole-screen contact sheets first and opened suspicious frames at
native resolution. Duplicates were collapsed only when their whole-frame
pixels matched. Crops and zooms were supporting views, never substitutes for
the full frame. Extracted single sprites and source panels were not counted as
gameplay screenshots. This was a visual review; I did not rerun tests or infer
Genesis fidelity from Debug/Release agreement.

## The questions applied

For each screen family, the review used the same questions and recorded the
applicable evidence:

1. **Does the whole frame look correct?** Inspect placement, palette, clipping,
   scaling, unexpected blank areas, and artifacts beyond the edited control.
2. **Does the image match the behavioral state?** Check selection, equipped or
   unequipped state, open/closed page, displayed values, and state transitions
   against controller/source state where available.
3. **Are the visuals themselves supported?** Compare geometry and appearance
   with ROM-derived placement and independent game captures; label internal
   render comparisons as internal consistency only.
4. **Should it animate?** Check multiple frames for cadence, duration, and the
   conditions that start or stop the animation.
5. **Is a sound tied to it?** Trace the request, clip identity, and duration;
   leave these unresolved when the evidence does not identify them.
6. **What else is visible in the frame?** Review the full screen for related
   state errors and visual defects outside the edited area.
7. **Do multiple examples support the reading?** Use adjacent or repeated
   states and independent captures when available; do not infer a whole
   animation from one still.
8. **Was the previous page cleared?** Check that old buttons, text, cursors,
   and background layers are hidden or replaced before the next page is drawn.

## Findings by screen family

### Findings from the expanded whole-frame pass

- **Archived opening mismatch:** the October 3 `wilderness-full-pass` captures
  in `decompilation/ghidra_conversion/build-ghidra-conversion-qt-debug/qt_runtime/`
  show projectiles crossing the three-line tri-vid narration in frames
  1050-1060. Independent section 02 frames 000638, 000660, and 000690 show
  the same narration state with projectiles below or beside the text. The
  old native output is a visible mismatch; the exact frame alignment and
  projectile coordinate/draw-order trace remain open. The October 5 source
  change touched QA allocation and field/hotel capture helpers, not the
  opening-scene renderer. There is no matching post-change whole-frame intro
  dump in the current output tree, so the mismatch is likely still present but
  is not yet confirmed against a fresh build. Re-capture and compare the
  current renderer before marking this resolved.
- **Stale broken trade image:**
  `decompilation/ghidra_conversion/build-ghidra-conversion-qt-debug/qt_runtime/field-action-qa/02_inventory_action_trade.bmp`
  is almost entirely black, with only fragments of the portrait, icons, and
  posture row. It is dated October 1 and predates the current source. The
  October 5 `field-inventory-trade-qa/actual.bmp` is a complete 320x224 frame.
  The old image is a broken archived artifact, not evidence that the current
  trade screen has that appearance.
- **Current unequipped inventory frames:** the October 5 `field-action-qa`
  whole frames for weapons 1 and 2 show the item icons and surrounding menu;
  I did not see the extra large black rectangle reported for the unequipped
  state. The selected icon retains the thin warm-gold selector. These are
  internal conversion captures; the exact equip/unequip transition still has
  no matching independent Genesis capture.
- **Hotel Options appearance remains open:** the current
  `field-hotel-qa/options/actual.bmp` is green option text on black. It is
  visually unusual beside the full Attributes/Skills composition, but no
  matching independent Hotel capture was identified. The Hotel Karma spend
  page and the Start-menu Karma status page are distinct full-screen images;
  the spend page adds the upgrade/cost detail row. Keep the two routes
  separate, and do not call the text-only Hotel Options appearance visually
  verified against Genesis.
- **Seattle focus outline is intentional:** the amber outline in the root
  `startup-qa/opening_604.bmp` through `opening_612.bmp` follows the five
  source-derived focus phases. The separate `seattle-outline-window` images
  are an older white-outline experiment and are not the expected product
  appearance. Do not classify the amber Seattle focus box as an old button
  drawn over the skyline.
- **Contaminated title captures:** the older `capture-title-boot` sequences
  contain video-player controls and creator overlays. Those pixels belong to
  the capture UI and were excluded from game-art comparisons.

The current field-action, Hotel child, field-secretary, Notebook, shop, and
menu frames were reviewed as full images, including surrounding HUD and page
replacement. No second unambiguous off-target overlay was identified in those
current outputs. That statement is limited to visible artifacts; it is not a
claim of independent Genesis fidelity.

### Pre-title, FASA, and title handoff

The regenerated whole-frame sequence is internally coherent: SEGA shock,
lightning, strobe, cleared lightning, figure fade, resolved SEGA, black,
FASA fade-in, full hold, FASA fade-out, black, and title handoff. The saved
state trace records FASA fade-in at source frames 371 and 373, full brightness
at 405, the last hold frame at 525, fade-out start at 526, first fade pass at
528, black at 558, and title handoff at 560. The full-frame FASA image is the
blue centered mark; no SEGA figure, lightning, menu controls, or old FASA
pixels survive the replacement. The first fade-in and fade-out passes each
change 1,948 pixels within the logo bounds. The full-hold and last-hold images
are pixel-identical.

The independent section 01 frames 210, 216, 240, 280, 290, and 300 support the
same dim/full/hold/dimming/black order. Their compressed 30-fps cadence does
not establish an exact offset against the 60-Hz source trace. The title
handoff capture at frame 560 shows the title circuit background before the
logo is revealed. It proves that FASA was replaced and no previous screen is
left underneath; it does **not** prove the title-logo timing or the completed
title screen. Independent frames 360 and 390 show partial and full title-logo
states, but the cross-capture timing remains open. Do not cite the frame-560
boundary image alone as visual verification of the full title screen.

FASA's install, fade, and hold path makes no direct sound request. The SEGA
path issues audio service requests, but their clip aliases and durations are
not isolated by the available capture. The whole pre-title hill therefore
remains partial pending independent timing and audio evidence.

The old root-level `boot-fasa-audit` images are stale: they show a bright FASA
mark at a frame that conflicts with the traced fade boundary. Keep them out of
current visual comparisons. The fresh `qt_runtime/boot-fasa-qa` images are the
current captures. The SEGA/FASA phase notes and source trace are in
[`qt-visual-audit.md`](../visual-analysis/qt-visual-audit.md).

### Title, start menu, and Karma routes

The whole-screen captures show the title/menu layers being replaced as the
phase changes, with no residual FASA or previous menu buttons over the next
page. The start-menu hover state does not open Karma by itself in the reviewed
captures. The start-menu Karma view and the hotel-spending Karma screen are
separate routes and were reviewed as such; they are not treated as one
reusable visual state. Exact title-logo timing at the entry boundary remains
open as described above. The screen-specific audit is
[`karma-status-and-hotel-upgrade.md`](karma-status-and-hotel-upgrade.md).

Older `capture-title-boot` images contain video-player controls and a creator
overlay. Those pixels are capture UI, not game art, and were excluded from
visual claims. The independent FFmpeg frames remain the cleaner comparison
source; creator text on early source frames is also excluded.

### Inventory, equipment, and selection

The complete inventory frames show the selected-cell outline as a thin yellow
border, with the icon visible inside it. In the unequipped weapon screenshots
there is no extra black rectangle around the item. Weapon 1's unequipped frame
matches a fresh baseline after a 30-frame idle hold. Weapon 2's equipped icon
and unequipped state also remain stable through their 30-frame holds. Debug
and Release field-action QA pass, and the screen-wide review found no leftover
selection graphics outside the active item cell.

The independent section 06 frames support inventory placement and the thin
selector, but do not capture the specific weapon equip/unequip transition.
So these captures establish the Qt state and clearing behavior, not a final
Genesis-fidelity verdict for that exact toggle. The posture bar is part of the
menu navigation state, including up/down focus and its attack/defense effect;
its targeted evidence is recorded in the previously committed posture audit.

### Hotel, dialogue, shop, HUD, and field transitions

The full frames were checked around page entry/exit and for old controls under
new ones. No additional whole-screen stale-button or doubled-panel defect was
found in these reviewed artifacts. Hotel/dialogue text stays within its
window, and its selected options are visible in the expected state. This is
not a claim that every dialogue string or shop item is independently verified
against the ROM.

Before the deterministic QA seed was added, Debug and Release Hotel captures
differed only in the lower shared bezel at `x=40..231`, `y=200..215`. Source
trace identifies these as the intentional twelve-cell random-grid effect
written by field routine `0xE660`; it is not an old button left underneath the
hotel UI. A fixed seed is now injected only by the Hotel screenshot QA path,
so matching Debug/Release images are reproducible. The live runtime still
initializes its seed normally. The field renderer's full-frame layering and
QA outputs are in
[`field_scene_runtime.cpp`](../../ghidra_conversion/qt_runtime/src/field_scene_runtime.cpp)
and the current `field-hotel-qa` output directories.

### Ares and older transition samples

The 19 Ares sample captures were inspected with their neighboring samples.
The one-second captures show field-to-black, dialogue entry, shop display,
and a later fade to black. They are too sparse to establish animation cadence
or prove that every black sample is a game-produced held frame. `frame-00`
and `frame-21` are black while adjacent captures show other screens; treat
them as capture-boundary/transition-ambiguous evidence. Do not use them alone
to claim an in-game black-screen defect.

## Whole-corpus result and remaining limits

The earlier conclusion that no new whole-frame distortion was found is
withdrawn. The expanded review found a visibly broken stale trade image and a
projectile/text mismatch in archived opening output. It also confirmed that
some surprising frames are capture or experiment artifacts, while the current
Hotel Options text-only screen has no independent visual reference. The
current source still needs a fresh full-frame intro capture before the
archived projectile mismatch can be called fixed or current.

The earlier 139-path Debug/Release pixel match applies only to the narrow
directory set listed above. Pixel-identical builds establish reproducibility,
not visual fidelity. The current field-action screenshots do not show the
reported large black unequipped-item box, but no independent Genesis capture
shows that exact equip/unequip transition. Exact title timing, the Hotel
Options appearance, Ares transition cadence, and several audio clip identities
remain unresolved. Do not turn internal screenshot agreement into an external
correctness claim.

## Capture stability fix

The following records the implementation and test results from the earlier
capture-stability work. They were not rerun during this expanded screenshot
review and do not resolve the visual findings above.

Debug FASA QA had been crashing while it constructed a second, large
`GhidraConversionWindow` on the stack during the outer window's QA method. The
secondary QA widget is now heap-allocated, avoiding the overlapping window
and typed source-state stack allocations. The Hotel QA path now uses an
explicit deterministic random seed so that its lower-bezel source effect does
not create unrelated build-to-build screenshot noise.

After these changes, the following focused QA cases passed in Debug and
Release:

- `shadowrun_ghidra_conversion_qt_boot_fasa_qa`
- `shadowrun_ghidra_conversion_qt_field_action_qa`
- `shadowrun_ghidra_conversion_qt_field_hotel_qa`
