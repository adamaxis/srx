# Ares Weapon Emporium verification

Status: controller, transaction logic, source-selected Ares mode-text records,
the twelve source field-service titles and their draw destinations, and a
text-only Qt prompt runtime with a diagnostic dump are implemented. The live
field event route, shop presenter, and live wallet binding are not connected.
The source uses a generic mode-text prompt here, not the Stoker/NPC panel
resolver. No Ares prompt or shop frame has been confirmed in the available
early-section capture, so no visual fidelity claim is made.

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
- Prompt draw: `0x1F246` loads the shared menu assets and font, prepares the
  optional mode resources, then calls `0x1F3C4`; `0x1F5A8` clears the source
  text and choice wells before the prompt draws. The Qt runtime uses the
  extracted common panel frame/font, splits source paragraph separators into
  text segments, draws no portrait or A/B/C marker for the sentinel record,
  and waits 30 frames before accepting a sampled controller edge. Whether the
  common panel frame is the exact Ares exterior remains unverified without an
  independent Ares image. The CLI dump is diagnostic; the live event is not
  connected yet.
- Dialogue boundary: NPC prompt resolver `0x1F65A` matches mode 2/event 2 for
  the Stoker motel. Ares mode 2/event 11 follows `0x1FA78`; it must use its own
  event-selected mode-text data and must not borrow Stoker speaker assets.
- Selection: `0x56AC0` consumes `FFDD22` direction/confirm/return edges.
  `0x58C54` bounds the selection cursor, `0x58CDE` switches the two panels,
  `0x587DA` publishes the action selection, and `0x57C7A` redraws the selected
  action presentation. The menu has no periodic animation in this loop.
- Title: `0x56AC0` indexes the 12-entry pointer table at `0x1AAB50` with
  `FFF0B6`. Ares uses entry 1, whose record at `0x1AAB92` stores destination
  cell `0xC110` and text `Ares Weapon Emporium`. The offline extractor now
  publishes every title and destination as host text in `menu/field/text.tsv`;
  the Qt runtime does not follow those ROM pointers.
- Mode-entry sprite: `0x1FA78` selects profile `0x1220C0`; its ROM pointers are
  palette `0x1219BC`, pattern stream `0x12C84E`, and sprite stream `0x12266A`.
  The offline importer emits the profile's 7x9 sprite as
  `field/mode-entry/ares-sprite.bmp` (56x72). This is one profile resource,
  not evidence for the shop's background or complete screen composition.
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

1. **Does it look correct?** The diagnostic prompt has clean text in the host
   panel. The extracted profile sprite is visible and nonblank at 56x72, but
   its screen placement and the exact Ares exterior remain unverified. The
   shop has no presenter; title extraction adds no visible pixels.
2. **Does appearance reflect behavior?** The title-only sentinel has no
   selection, speaker, portrait, or A/B/C choice. Runtime tests check those
   absences, event/result text selection, and the 30-frame input gate. The
   profile sprite is selected by the Ares route and carries its source palette
   attributes; the shop cursor has not been rendered.
3. **Are visuals correct?** ROM prompt text and well preparation are traced;
   Debug produced and inspected
   `field-mode-text-qa/introduction/actual.bmp`. The Ares title and `0xC110`
   draw destination have source assertions and Debug/Release checks. The new
   profile sprite is generated by the offline importer and its 7x9/56x72
   shape and nonempty pixels are checked in Debug and Release. These host
   outputs are not independent Genesis goldens. Shop pixels remain pending.
4. **Animation?** The prompt has no periodic animation; it waits 30 frames
   before accepting a controller edge. The mode-entry sprite extraction is a
   single static resource; its later animation behavior is not established.
   The shop loop redraws after input and has no periodic shop animation.
5. **Sound?** The traced `0x1F246`/`0x1F3C4` prompt path makes no direct sound
   request. The shop requests event `0x25` on cursor/panel movement and `0x24`
   on confirmation and runner restart. The `0x25` clip identity and playback
   durations remain unresolved; shop audio is not connected here. Title
   extraction has no sound behavior.
6. **Related screen content?** The prompt redraws the complete host panel
   before the shop. The profile sprite is not drawn by the runtime yet, so its
   overlap with other screen content cannot be judged. The eventual shop view
   must compose its title, active-runner HUD, selected action detail, offer
   panel, and funds/price feedback together.
7. **Multiple frames?** The available longplay has no Ares prompt or shop
   example. No independent frame is available to compare; the diagnostic dump
   is one host frame, not another game example.
8. **Stale UI?** The prompt redraws its full host frame and clears both text
   wells. Its test verifies the lower choice well contains no old buttons. No
   shop runtime panel has been added in this slice; shop entry, panel changes,
   confirmations, and return still need full-panel redraw and pixel checks.

## Implemented controller slice

`ShopMenuController` wraps the existing typed field-service selection state and
returns purchase, sale, source-boundary, runner-change, and field-return
requests. It does not own a second inventory or wallet. `ShopTransactionService`
delegates to the existing typed purchase/removal conversions. The focused test
checks Ares stock order, navigation, confirmations, protected sale rows,
funds, and inventory mutations in Debug and Release. The offline importer now
exports the 12 title strings and source draw cells under the named field-menu
text asset; the Ares record is checked against `0x1AAB92`/`0xC110`. This data
slice does not render or connect the live shop. The offline importer also
exports profile `0x1220C0` as the named 56x72 `field/mode-entry/ares-sprite.bmp`
asset; this is only one sprite resource and is not used by the Qt runtime yet.
`FieldModeTextRuntime` displays the three ROM-selected text records in a
no-choice panel, enforces the sentinel delay, and writes an `actual.bmp`
diagnostic. The Ares hill remains partial until the live field route, shop
presentation, wallet binding, stale shop-panel check, and independent visual
evidence are complete. The exact Ares-specific frame and background remain
unverified.
