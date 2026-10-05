# Full screenshot review — 2026-10-05

## Scope and method

This pass reviewed the complete screenshot artifact sets present in the
checkout, including full frames outside the code areas changed in this session.
I reviewed whole-screen contact sheets first, opened suspicious and newly
generated frames at native resolution, and used SHA-256 hashes to compare
matching Debug and Release outputs. Crops and zooms were treated as supporting
views, never as substitutes for the full frame. Extracted single sprites and
source panels were not counted as gameplay screenshots.

The Qt runtime trees contained 152 BMP/PNG files in Debug and 139 in Release
after the FASA recapture. There are 139 matching relative paths; all 139 are
pixel-identical. Debug has 13 additional diagnostic images and Release has no
Release-only images. These directories include generated review aids, so the
unique pixel-image counts are 93 in Debug and 82 in Release. The newly
regenerated FASA QA set contains 15 full-frame images in each configuration;
each Debug/Release pair is identical, and every Debug image was opened at its
original 320x224 resolution.

The older screenshot inventory included 730 BMP/PNG candidates across:

- `build-qt-debug/capture-title-boot`, `capture-title-boot-2`,
  `cab-map-tile-verified`, `menu-anomaly-inspection`,
  `menu-selector-verified`, and `visual-regression`;
- `build-debug/visual-regression`, `build-release/visual-regression`,
  `build-debug/ares-video-samples`, and `build-debug/dialogue-buttons`;
- selected top-level full-frame captures in those build trees.

Those files reduce to 437 unique pixel images. The nine root-level images in
`build-qt-runtime-debug/boot-fasa-audit` were also reviewed. The Ares sample
set contains 19 unique captures. The clean gameplay reference was sampled at
18 full frames in section 01, including the SEGA, FASA, title-logo, and
copyright phases. This is a review of the screenshot artifacts in the listed
sets and the named independent reference samples; it is not a claim that every
frame in the longplay video was individually inspected.

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

No new off-target whole-frame distortion was identified in the reviewed
corpus. The two concrete review issues were (a) stale root-level FASA audit
captures, now explicitly retired from comparison, and (b) nondeterministic
lower-bezel pixels in QA, now fixed with a QA-only seed. Full-frame views also
prevent the previous mistake of treating an acceptable crop as proof that the
rest of the screen was correct.

All 139 matching Debug/Release screenshot paths are now pixel-identical. This
proves reproducibility between those builds, not visual fidelity by itself.
Independent visual confidence varies by scene; the specific weapon toggle,
exact title timing, Ares transition cadence, and several audio clip identities
remain unresolved. Continue to label those as open rather than converting
internal screenshot agreement into an external correctness claim.

## Capture stability fix

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
