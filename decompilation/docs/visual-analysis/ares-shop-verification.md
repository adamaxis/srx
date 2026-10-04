# Ares Weapon Emporium verification

Status: controller and transaction logic are partial. The Qt field event route,
generic event dialogue, presenter, and live wallet binding are not connected.
No independent Ares shop frame occurs in the supplied early-game longplay
capture, so no visual fidelity claim is made.

## Source trace

- Entry: US ROM `0x1FA78`, scene 2/submode 11, sets menu mode 1 and menu result
  -1, runs mode-exit services, then dispatches `0x56AC0`.
- Selection: `0x56AC0` consumes `FFDD22` direction/confirm/return edges.
  `0x58C54` bounds the selection cursor, `0x58CDE` switches the two panels,
  `0x587DA` publishes the action selection, and `0x57C7A` redraws the selected
  action presentation. The menu has no periodic animation in this loop.
- Offers: `0x56DC4` chooses one of three action tables. Ares mode 1 selects
  three-byte rows at `0x1AAC9E`, `0x1AADD7`, and `0x1AAD8F`; `0xFF` in byte 0
  ends each offer list. Those rows are represented as typed records in
  `shop_menu_runtime.cpp`.
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
6. **Related screen content?** The field-mode service also redraws its title,
   active-runner HUD, selected action detail, offer panel, and funds/price
   feedback. The current runtime does not yet compose these regions together.
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
checks Ares stock, navigation, confirmations, protected sale rows, funds, and
inventory mutations in Debug and Release. The Ares hill remains partial until
field routing, presentation, dump, stale-panel check, and visual evidence are
complete.
