# Ares shop category and offers-panel review

**Status:** The Ares selector follows the ROM category order and redraw
behavior. Categories are Weapons, Armor, and Equipment. Moving between
categories retains the previously rendered stock row until the offers panel
opens. The full all-category/item/cancel traversal remains open under
`SHOP-01`.

## Reproduction and full-screen captures

Release CTest
`shadowrun_ghidra_conversion_qt_field_ares_collision_shop_qa` uses the ordinary
New Game route, physically contacts Scene 2 collision record 92, waits through
event 11's Ares prompt, and accepts it with A. The QA pins the random stream to
zero. It saves the entry category, presses Right once to Armor, presses Right
again to Equipment, then presses Down to open the first offer.

- Before the redraw fix, after Right: `decompilation/ghidra_conversion/build/conversion/qt/release/qt_runtime/l5-ares-category-review/before/ares-shop-other-items-category.bmp`.
- After fix, Weapons: `decompilation/ghidra_conversion/build/conversion/qt/release/qt_runtime/field-ares-collision-shop-qa/ares-shop-category.bmp`.
- After fix, Armor after one Right with the prior stock row retained: `decompilation/ghidra_conversion/build/conversion/qt/release/qt_runtime/field-ares-collision-shop-qa/ares-shop-armor-category.bmp`.
- After fix, Equipment after the second Right, still with the prior stock row retained: `decompilation/ghidra_conversion/build/conversion/qt/release/qt_runtime/field-ares-collision-shop-qa/ares-shop-other-items-category.bmp`.
- After fix, Down enters the Equipment offers panel and displays Universal Gun Clips: `decompilation/ghidra_conversion/build/conversion/qt/release/qt_runtime/field-ares-collision-shop-qa/ares-shop-universal-clips-detail.bmp`.

Each capture is a complete 320x224 screen including the Ares frame, controls,
right-side runner panel, stock row, and wallet. The pre-fix image came from the
Release test run that failed the stock-row assertion. The post-fix images came
from the passing ordinary-input route. A previous side-by-side review is in
`decompilation/ghidra_conversion/build/conversion/qt/release/qt_runtime/l5-ares-category-review/longplay-sequence-left_release-right.png`.

Independent longplay frames in section 25 show three of these states:

- `video analysis/long_gameplay_video_rest_ffmpeg_30fps_320x224/section_25_source_5820-6120/frame_004950.png`: Weapons is selected and its offers are visible.
- `.../frame_004980.png`: Equipment is selected, but the Weapons stock row is still visible.
- `.../frame_005010.png`: Universal Gun Clips is selected and its description replaces the empty detail well.

The sampled frames establish the visuals, but not the number of category
inputs between them; intervening frames may omit Armor. The ROM table below
establishes the order and cursor values. The second-to-third visual transition
also shows the tab focus changing before the stock row and detail are redrawn
when the offers panel opens.

## ROM trace

The primary source is the US ROM
`input/shadowrun_genesis_us.bin`. I read its instruction bytes directly at
`0x56AC0`, `0x56DC4`, and `0x58B5E..0x58D7C`; Ghidra disassembly at those
addresses is only a cross-check.

- At `0x56DC4`, action cursor 0 selects the mode-1 table at `0x1AAC86`, cursor
  1 selects `0x1AADBF`, and cursor 2 selects `0x1AAD77`. The heading rows at
  `0x1AA8AF + cursor * 0x2A` label them Weapons, Armor, and Equipment.
- For mode 1, cursor 1's table contains offer types `0x2D`, `0x2F`, and
  `0x30`; cursor 2's table contains `0x45/0x19`, `0x0F/0x06`,
  `0x10/0x04/0x07`, and `0x11/0x04/0x07`. The host retains the existing
  `other_items` identifier for the source's Equipment records.
- `0x56AC0` gives the category cursor and offers-panel cursor separate input
  paths: primary-cursor movement reaches `0x58C54`, while panel movement
  reaches `0x58CDE`.
- `0x58C54` updates category selection and, while panel 0 is active, refreshes
  the tab/title selection through `0x58B5E` and `0x58BB6`. It does not call the
  selected-offer detail renderer `0x57C7A`, so the current stock row remains.
- `0x58CDE` changes the panel and resets its row cursor. Entering panel 1 calls
  `0x58B5E` and `0x58D32`; the latter rebuilds selector records and calls
  `0x57C7A` to render the newly selected offer. Returning to panel 0 redraws
  the tab/title area without `0x57C7A`, retaining that row until a later
  offers-panel entry.

The host retains typed `displayed_offer_group` state for the stock art and
updates it only when the player enters the offers panel. Category focus and
the last rendered offer row therefore remain separate visible facts. The
selected category and offer still drive transactions.

## Failed test, fix, and result

The presenter test captures the full screen after category movement and
requires the tab and heading to change while the 240x24 stock row remains the
Weapons row. Before the fix, the cursor was on Equipment but the presenter had
already swapped the stock icons to patches and medical items. The pre-fix
frame above records that mismatch.

The fix retains the last rendered stock group until the panel transition.
Debug and Release CTests pass. The shop-menu runtime test checks that category
focus can move independently of displayed offers, entering the offers panel
updates the displayed group, and returning to categories retains the last
offer row until the next panel selection.

## Eight-question visual review

1. **Does it look correct?** Armor and Equipment category screens retain the
   Weapons icons shown in the independent longplay. Down then changes to the
   Universal Gun Clips row and detail.
2. **Does the look reflect behavior?** The first Right focuses Armor; the
   second focuses Equipment while the offers panel stays inactive. Down opens
   Equipment offers at cursor 0 and displays its detail.
3. **Are the visuals correct?** The tab highlight, fixed-width heading, stock
   row, clips icon, text well, and slim first-item selector match these source
   states. The longplay has another portrait and 7,923 nuyen; this New Game
   route has Joshua and 20, so those HUD pixels should differ. Video capture
   changes palette edges and pixels; there is no source evidence to change the
   extracted service-background colors.
4. **Is it animated?** The category and detail pages are static between input
   edges. The 30-frame source-menu hold in the shop QA produces no category
   animation or repeated navigation request.
5. **Is there a sound?** Each accepted category or offer movement requests
   source event `0x25`. It resolves to `audio.effects.resource.037`, whose
   decoded file lasts 2.134 seconds including its two-second release tail.
   The route records requests; independent audio QA covers playback and
   duration.
6. **Are related areas correct?** The complete screen was reviewed. The shop
   frame, tabs, row, detail panel, wallet, and right-side runner panel remain
   present through the sequence.
7. **Do multiple frames support this reading?** Independent longplay frames
   show Weapons, Equipment with the old row, and then the selected clips
   offer. The Release route saves Weapons, the intervening Armor category,
   Equipment, and the offer as complete screens.
8. **Are controls drawn over old UI?** Shop entry paints the full service
   background. The Ares prompt and field map are absent. The retained stock
   row is current menu content; no dialogue, map controls, or unrelated
   buttons remain.

This closes the initial category/side-panel mismatch. Other shop categories,
item-row cursor movement, cancel behavior, unaffordable feedback, and the
remaining menu rows in `luna_handoff.md` still need their own reviews.
