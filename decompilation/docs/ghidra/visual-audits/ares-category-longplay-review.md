# Ares shop category and offers-panel review

**Status:** The initial Weapons → Equipment → first-offer sequence now follows
the ROM's redraw behavior. The host used to replace the stock row as soon as
the category cursor moved. The ROM keeps the previously rendered stock row
until the player enters the offers panel. The full all-category/item/cancel
traversal remains open under `SHOP-01`.

## Reproduction and full-screen captures

Release CTest
`shadowrun_ghidra_conversion_qt_field_ares_collision_shop_qa` uses the ordinary
New Game route, physically contacts Scene 2 collision record 92, waits through
event 11's Ares prompt, and accepts it with A. The QA pins the random stream to
zero. In the shop it saves the entry category, presses Right once, and presses
Down once.

- Before fix, after Right: `decompilation/ghidra_conversion/build/conversion/qt/release/qt_runtime/l5-ares-category-review/before/ares-shop-other-items-category.bmp`.
- After fix, Weapons: `decompilation/ghidra_conversion/build/conversion/qt/release/qt_runtime/field-ares-collision-shop-qa/ares-shop-category.bmp`.
- After fix, Equipment category with the prior stock row retained: `decompilation/ghidra_conversion/build/conversion/qt/release/qt_runtime/field-ares-collision-shop-qa/ares-shop-other-items-category.bmp`.
- After fix, Down enters the other-items offers panel and displays Universal Gun Clips: `decompilation/ghidra_conversion/build/conversion/qt/release/qt_runtime/field-ares-collision-shop-qa/ares-shop-universal-clips-detail.bmp`.

Every file is a complete 320x224 screen including the Ares frame, controls,
right-side runner panel, stock row, and wallet. The pre-fix image was captured
from the Release test run that failed the new stock-row assertion. The
post-fix images came from the passing ordinary-input route. The side-by-side
review is in `decompilation/ghidra_conversion/build/conversion/qt/release/qt_runtime/l5-ares-category-review/longplay-sequence-left_release-right.png`.

Independent longplay frames in section 25 show the same three states:

- `video analysis/long_gameplay_video_rest_ffmpeg_30fps_320x224/section_25_source_5820-6120/frame_004950.png`: Weapons is selected and the Weapons offers are visible.
- `.../frame_004980.png`: Equipment is selected, but the Weapons stock row is still visible.
- `.../frame_005010.png`: the first other-items offer, Universal Gun Clips, is selected and its description replaces the empty detail well.

The transition between the second and third frames is the key evidence: the
heading/tab focus changes first; the stock row and offer detail change when
the offers panel opens.

## ROM trace

The primary source is the US ROM
`input/shadowrun_genesis_us.bin`. I read its instruction bytes
directly at `0x56AC0` and `0x58B5E..0x58D7C`; the Ghidra disassembly at the same
addresses is only a cross-check.

- `0x56AC0` gives the category cursor and offers-panel cursor separate input
  paths: primary-cursor movement reaches `0x58C54`, while panel movement
  reaches `0x58CDE`.
- `0x58C54` updates the category selection and, when panel 0 is active,
  refreshes the tab/title selection through `0x58B5E` and `0x58BB6`. It does
  not call the selected-offer detail renderer `0x57C7A`, so the existing stock
  row remains on screen.
- `0x58CDE` changes the panel and resets its row cursor. Entering panel 1 calls
  `0x58B5E` and `0x58D32`; the latter rebuilds the selector records and calls
  `0x57C7A` to render the newly selected offer. Returning to panel 0 redraws
  the tab/title area without calling `0x57C7A`, so that stock row remains until
  a later offer-panel entry.

The host now retains a typed `displayed_offer_group` for the stock art and
updates it only when the player enters the offers panel. Category focus and
the last rendered offer row therefore remain separate visible facts. The
existing shop controller still supplies the selected category and selected
offer to transactions.

## Failed test, fix, and result

I added the test before changing the presenter. It captures the full screen
after Right and requires the tab and heading to change while the 240x24 stock
row stays unchanged from the Weapons screen. Release CTest failed on that
assertion: the cursor was on Equipment, but the presenter had already swapped
the stock icons to patches and medical items. The pre-fix frame above records
that mismatch.

The fix retains the last rendered stock group until the panel transition.
Afterward the Release and Debug CTests pass. The shop-menu runtime test also
checks that category focus can move independently of displayed offers, that
entering the panel updates the displayed group, and that returning to the
category panel retains the last offer row until the next panel selection.

## Eight-question visual review

1. **Does it look correct?** The post-fix Equipment screen now has the same
   Weapons icons shown in the independent longplay. Down then changes to the
   Universal Gun Clips row/detail.
2. **Does the look reflect behavior?** Yes. After Right, the Equipment tab is
   focused while the offers panel remains inactive. After Down, the other-
   items offer panel is active with cursor 0 and its detail is shown.
3. **Are the visuals correct?** The tab highlight, fixed-width heading, stock
   row, clips icon, text well, and slim first-item selector match the three
   source states. The longplay save has another portrait and 7,923 nuyen;
   this New Game route has Joshua and 20, so those HUD pixels should differ.
   Video capture also changes palette edges and pixels; no source evidence
   supports changing the extracted service-background colors.
4. **Is it animated?** The shop category and detail pages are static between
   input edges. The 30-frame source-menu hold in the existing shop QA produces
   no category animation or repeated navigation request.
5. **Is there a sound?** Right and Down each request source event `0x25` once.
   It resolves to `audio.effects.resource.037`, whose decoded file lasts 2.134
   seconds including its two-second release tail. The route records requests;
   independent audio QA covers playback and duration.
6. **Are related areas correct?** The complete screen was reviewed. The shop
   frame, tabs, row, detail panel, wallet, and right-side runner panel remain
   present through the sequence.
7. **Do multiple frames support this reading?** Yes. The three adjacent
   independent frames show Weapons, Equipment with the old row, then the
   selected clips offer. The Release route saves the corresponding three
   complete screens.
8. **Are controls drawn over old UI?** Shop entry paints the full service
   background. The generic Ares prompt and field map are absent. The retained
   stock row is the source's current displayed menu content; no old dialogue,
   map controls, or unrelated buttons remain.

This closes the reported initial category/side-panel mismatch. Other shop
categories, item-row cursor movement, cancel behavior, unaffordable feedback,
and the remaining menu rows in `luna_handoff.md` still need their own reviews.
