# Ares Weapon Emporium verification

Status: controller, transaction logic, and the source-selected Ares mode-text
record provider are partial. The Qt field event route, prompt presenter, and
live wallet binding are not connected. The source uses a generic mode-text
prompt here, not the Stoker/NPC panel resolver. No Ares shop frame has been
confirmed in the available early-section capture, so no visual fidelity claim
is made.

## Source trace

- Entry: US ROM `0x1FA78`, field mode `FFFBF8=2`/event `FFFBF9=11`, selects
  mode resource `0x1220C0`, sets menu mode 1 and result -1, and selects text
  record `0x110DD2`. It calls `0x1F246` (whose `0x1F3C4` child handles the
  generic mode-text prompt), then dispatches `0x56AC0`.
- Prompt data: record `0x110DD2` points to text at `0x118934`, followed by the
  `0xFFFFFFFF` first-option sentinel. The prompt draws no A/B/C choices; ROM
  `0x1F3C4` waits 30 frames and returns `0xFB` on any controller event before
  the default route enters the shop. Positive `FFF0D5` selects record
  `0x111676` / text `0x118A6C` (package payment); negative selects
  `0x11166E` / text `0x1189F2` (package handoff). Those alternate paths return
  without entering `0x56AC0`. The native provider keys all three text records
  by mode/event/result and does not assign them Stoker's speaker or portrait.
- Dialogue boundary: NPC prompt resolver `0x1F65A` matches mode 2/event 2 for
  the Stoker motel. Ares mode 2/event 11 follows `0x1FA78`; it must use its own
  event-selected mode-text data and must not borrow Stoker speaker assets.
- Selection: `0x56AC0` consumes `FFDD22` direction/confirm/return edges.
  `0x58C54` bounds the selection cursor, `0x58CDE` switches the two panels,
  `0x587DA` publishes the action selection, and `0x57C7A` redraws the selected
  action presentation. The menu has no periodic animation in this loop.
- Offers: `0x56DC4` chooses one of three action tables. Source action indices
  0, 1, and 2 map to weapons, other items, and armor at `0x1AAC9E`, `0x1AADD7`,
  and `0x1AAD8F`. Direct US ROM reads confirm item codes `01,02,06,04,05,0D,0E`,
  `45,0F,10,11`, and `2E,2D` respectively. `0xFF` in byte 0 ends each offer
  list. Those rows are represented as typed records in `shop_menu_runtime.cpp`.
- Purchase: `0x5794E` applies a selected offer; `0x57A44` reads its full word
  base cost from `0xD4540 + type * 0x30 + 0x22`; `0x57A6E` applies the mode and
  runner adjustment; `0x57B8C` confirms before inventory or funds mutate.
- Sale: action selection 5 reaches `0x56F4A`; the selected action is quoted at
  half its item base cost with the runner surcharge, then removed after the
  shared confirmation. Codes `0x29..0x2B` are protected.
- Exit: Start restores the field asset sequence, field animation assets, and
  field setup at `0x56DA8..0x56DC2`.

## Visual audit questions

1. **Does it look correct?** Pending. There is no independent Ares capture, and
   the native presenter is not implemented.
2. **Does appearance reflect behavior?** The controller test checks the typed
   selected group, offer cursor, sell slot, and exact stock rows. A visible
   selector has not been rendered or checked.
3. **Are visuals correct?** Pending source-positioned presenter and `actual.bmp`.
   ROM draw entry points are traced above; the native screen is not built.
4. **Animation?** The menu loop redraws after input and has no periodic shop
   animation. Its behavior-counter display service is not a shop animation
   clock. Ares-specific cadence remains visually unverified.
5. **Sound?** The ROM requests event `0x25` on cursor/panel movement and `0x24`
   on confirmation and runner restart. The `0x25` clip identity and both
   playback durations are unresolved; Qt playback is not connected here.
6. **Related screen content?** The mode-text prompt precedes the shop and the
   field-mode service redraws its title, active-runner HUD, selected action
   detail, offer panel, and funds/price feedback. The current runtime does not
   yet compose these regions together.
7. **Multiple frames?** The available longplay has no Ares shop example. There
   are no independent shop frames to compare; the new controller CTest checks
   transitions, not appearance.
8. **Stale UI?** The presenter is pending. It must replace its complete owned
   panel on entry, panel switch, selection change, confirmation, and return;
   no controls from the prior page may remain underneath. A stale-panel pixel
   test is still required.

## Implemented controller slice

`ShopMenuController` wraps the existing typed field-service selection state and
returns purchase, sale, source-boundary, runner-change, and field-return
requests. It does not own a second inventory or wallet. `ShopTransactionService`
delegates to the existing typed purchase/removal conversions. The focused test
checks Ares stock order, navigation, confirmations, protected sale rows,
funds, and inventory mutations in Debug and Release. The Ares hill remains partial until
field routing, presentation, dump, stale-panel check, and visual evidence are
complete. `ModeTextPromptCatalogConversion` supplies the three ROM-selected
Ares text records and the no-option sentinel descriptor; it does not yet draw
the mode-specific background or connect the prompt return to the shop.
