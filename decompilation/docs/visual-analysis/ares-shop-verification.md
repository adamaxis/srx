# Ares Weapon Emporium verification

Status: controller, transaction logic, source-selected Ares mode-text records,
the twelve source field-service titles and their draw destinations, and a Qt
generic prompt runtime with the selected Ares mode-profile sprite are
implemented. The ROM-derived static field-service background is now available
as a named host asset. The Ares introduction was independently confirmed in
the full longplay at 01:39:43; the shop follows at 01:39:45. The live field
event route, shop presenter, and live wallet binding are not connected, so the
Ares hill remains partial.

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
  text and choice wells before the prompt draws. `0x1FA78` selects profile
  `0x1220C0` before the generic prompt; `0x1F292` uploads that profile sprite.
  The Qt runtime uses the extracted common panel frame/font and the named
  `field.mode.entry.ares.sprite` asset in the left speaker well. It keeps the
  sentinel record title-only (no A/B/C marker) and waits 30 frames before
  accepting a sampled controller edge. Its independent visual reference is
  `reference-frames/ares-introduction-5983.png`, decoded from the 01:39:43
  full-longplay frame and scaled to the native 320x224 presentation size. It
  shows the logo, text well, empty lower well, and full-panel redraw. The live
  field event route is still not connected.
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
- Service background: `0x56AC0` begins by calling `0x10BC4`. That routine
  loads the shared pattern stream at `0xCAB0C` through `0x10918`, copies the
  raw `0x340` bytes at `0xD263C` to VRAM `0x9F40`, then writes eight literal
  name-table blocks: Plane A `CE2A6/E000/34x2`, `CE32E/E100/2x13`,
  `CE362/E142/1x13`, `CE37C/E780/34x13`, `CE798/E744/6x14`,
  `CE840/E2C6/4x1`, and `CE848/E546/4x1`; Plane B `CE6F0/C044/6x14`.
  The checked name words use palette line 0 and low priority. The offline
  importer flattens the transparent Plane B backing and Plane A frame into
  `menu/field/service-background.bmp`; the runtime receives only this RGBA
  asset, not the ROM maps or VDP operations.
- Mode-entry sprite: `0x1FA78` selects profile `0x1220C0`; its ROM pointers are
  palette `0x1219BC`, pattern stream `0x12C84E`, and sprite stream `0x12266A`.
  The offline importer emits the profile's 7x9 sprite as
  `field/mode-entry/ares-sprite.bmp` (56x72). The longplay confirms it in the
  generic mode-text prompt's left well; it is not the shop's background or
  complete screen composition.
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

1. **Does it look correct?** The independent 01:39:43 capture shows the Ares
   logo in the left well, green text in the right well, and a blank lower
   well. The corrected Qt dump was inspected beside that frame and matches
   the placement and composition. The shop is a separate screen and still has
   no presenter.
2. **Does appearance reflect behavior?** Mode 2/event 11 selects profile
   `0x1220C0`; its Ares sprite is now bound to all three event-selected text
   results. The normal store-introduction sentinel has no choices or selection
   marker. Runtime checks cover its profile alias, visible left-well art,
   event/result text selection, and 30-frame input gate.
3. **Are visuals correct?** ROM prompt text, shared frame/well clears, selected
   mode profile, and sprite extraction are traced. Debug produced and inspected
   `field-mode-text-qa/introduction/actual.bmp` beside the independent
   `reference-frames/ares-introduction-5983.png`. The host dump checks the
   conversion; the longplay frame remains the visual authority. The new static
   shop background was inspected beside
   `reference-frames/ares-shop-5985.png`: the upper service frame, lower item
   panel, and right HUD backing occupy the expected regions. The source background
   does not include changing labels, item icons, selected-item focus, cursor, or
   portrait sprites; those shop pixels remain pending in the presenter.
4. **Animation?** The prompt has no periodic animation; it waits 30 frames
   before accepting a controller edge. The selected profile contributes one
   static sprite. Fifteen consecutive 30 fps frames around 01:39:54 show the
   shop selection and screen content unchanged; this supports a static shop
   background, with input-driven redraws still to implement.
5. **Sound?** The traced `0x1F246`/`0x1F3C4` prompt path makes no direct sound
   request. The shop requests event `0x25` on cursor/panel movement and `0x24`
   on confirmation and runner restart. The `0x25` clip identity and playback
   durations remain unresolved; shop audio is not connected here. Title
   extraction has no sound behavior.
6. **Related screen content?** The Ares prompt is a full-screen composition:
   the shared frame clears the prior field image, draws the selected profile
   in the left well, and leaves its lower well blank. The transition at
   01:39:44 is black; the shop at 01:39:45 then shows its own title, item
   detail, item row, and funds display while preserving the right-side field
   HUD. The extracted full-screen base redraw contains the service panel and
   right HUD backing as one named image; the presenter must redraw its owned
   dynamic rows over that base and clear them on return.
7. **Multiple frames?** The reference sequence includes the Ares introduction
   at 01:39:43, the shop category changes at 01:39:45-46, Universal Gun Clips
   at 01:39:47, Stim and Trauma Patch detail at 01:39:48-49, and a transaction
   prompt at 01:39:52. More item/category states continue through 01:40:00. At
   01:39:53.9, fifteen consecutive frames show a stable shop view. These
   longplay captures have no uploader overlay.
8. **Stale UI?** The introduction frame covers the full screen and clears both
   text wells; its lower well is black and contains no previous controls. The
   black transition at 01:39:44 separates the introduction from the shop at
   01:39:45, which replaces the introduction with its own interface. The shop
   static service background covers the whole 320x224 screen and therefore
   replaces the prior prompt pixels when used as a base. Presenter entry/return
   checks for dynamic shop labels, icons, focus, and confirmation controls remain
   pending.

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
asset. `FieldModeTextRuntime` displays the three ROM-selected text records
through the shared generic panel, binds the profile sprite to the Ares logo
well, enforces the sentinel delay, and writes an `actual.bmp` diagnostic. The
offline importer now emits `menu/field/service-background.bmp` from the eight
`0x10BC4` map writes and the shared pattern/palette source. Its test checks the
320x224 opaque host image contract; the independent longplay at
`reference-frames/ares-shop-5985.png` remains the visual authority. The Ares
hill remains partial until the live field route, shop presentation, wallet
binding, dynamic-row redraw, and stale-panel checks are complete. Shop sound
clip identity and duration also remain unresolved.
