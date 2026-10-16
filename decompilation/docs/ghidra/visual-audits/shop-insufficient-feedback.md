# Ares insufficient-funds feedback wait

## Reproduction and result

Open Ares, select Universal Gun Clips, move to the Predator offer, and press A
with 20 nuyen. The rejected purchase displays the insufficient-funds message.
In the old host behavior, the same still-held A input dismissed that message on
the next update. The Release QA failed before the fix with `waiting=0`,
`message=0`, and a changed frame at update 1.

The regression now keeps A held for 60 updates, releases it without a new press,
then sends a fresh A press. The message remains unchanged during all 60 held
updates and after release; only the fresh press dismisses it. The wallet remains
20, the selected offer remains at cursor 2, inventory is unchanged, and the
audio-request count stays at 4. The initial purchase action requested event
`0x24`; no additional sound request occurs during the wait. This verifies
request count, not the audible sample duration or Genesis chip playback.

Before-fix full-screen captures:

- `decompilation/ghidra_conversion/build/conversion/qt/release/qt_runtime/l5b-shop-feedback/before/frame-000-purchase-edge.bmp`
- `decompilation/ghidra_conversion/build/conversion/qt/release/qt_runtime/l5b-shop-feedback/before/frame-001-held-a.bmp`
- `decompilation/ghidra_conversion/build/conversion/qt/release/qt_runtime/l5b-shop-feedback/before/timeline.tsv`

After-fix full-screen captures and the 60-frame series:

- `decompilation/ghidra_conversion/build/conversion/qt/{debug,release}/qt_runtime/field-shop-ares-template-qa/shop-insufficient-feedback/`
- Key frames: `purchase-edge-0.bmp`, `held-1.bmp`, `held-60.bmp`,
  `released-0.bmp`, `fresh-edge-dismissed-0.bmp`; `timeline.tsv` records
  input, message state, funds, cursor, audio requests, and frame path.
- Each BMP is 320x224. All 64 Debug/Release artifact hashes match; the 60
  held frames have identical hashes to the initial feedback frame.

## ROM trace and implementation

The selected Ares action routine at ROM `0x5794E` branches to the common text
renderer at `0xC0DA` for no-slot and insufficient-funds feedback. Both branches
then jump to the input wait at `0xD94` (the insufficient-funds branch is at
`0x579E0`). That loop calls the controller sampler at `0x195C`, reads the
published controller event word at `FFDD22`, masks to its high byte, and loops
while no newly pressed button exists. It has no timer. A held button's low-byte
state is not a dismissal event. The decoded Ghidra function
`export/ghidra-c/fn_0005794E.cpp` agrees as a cross-check.

`ShopMenuController::advance_transaction` had accepted any nonzero controller
event while waiting, so a held low-byte A was mistaken for a new press. It now
requires the high-byte new-press event, matching the ROM input word. The unit
regression checks that a low-byte A leaves the message waiting and a new
high-byte A dismisses it. The field Ares QA additionally reproduces the full
input sequence and preserves every full-screen frame.

## Evidence limits and visual review

The targeted longplay contact sheets around 00:25:28–00:25:41 show a successful
Ares purchase/confirmation around 00:25:37–00:25:39. There is no independent
longplay frame showing an unaffordable-purchase page, so the exact failure text
layout and wait duration are not claimed as independently visually verified.
The before/after images are host captures used to demonstrate the reproduced
regression and corrected host behavior, not Genesis visual goldens.

The full-screen review checks the seven visual questions and the clean-redraw
rule:

1. **Does it look correct?** The feedback frame retains the shop screen and
   replaces the item detail with the rejection message.
2. **Does appearance match state?** Message, waiting flag, wallet, cursor,
   inventory, and audio request count are traced together.
3. **Are visuals correct?** The screen remains 320x224; no unsupported claim is
   made about exact Genesis text pixels without an independent failed-purchase
   frame.
4. **Animation:** The traced ROM wait is event-driven and has no animation or
   timeout. Sixty held frames remain stable.
5. **Sound:** The purchase cue event `0x24` is requested once; no rejection-wait
   request or additional repeated cue is present in the converted path.
6. **Related screen state:** The field background, shop title, Predator art,
   offer row, selected-row outline, HUD, and wallet remain present. Funds and
   inventory do not change.
7. **Multiple frames:** Purchase edge, every held update, release without a
   press, and fresh-press dismissal are captured; all 60 held frames are
   identical.
8. **No overdraw/stale controls:** Rejected feedback replaces the detail text;
   it does not stack over the detail text. Existing offer art, row selector,
   and HUD remain, and no stale dialogue or child-menu controls appear.

The focused `shadowrun_ghidra_shop_menu_runtime` and
`shadowrun_ghidra_conversion_qt_field_shop_qa` tests pass in Debug and Release
after the change. SHOP-02 is resolved for this specific held-input feedback
timing defect. A ROM-backed text-pixel comparison of the rejected-purchase
screen remains open because the longplay does not show that state.
