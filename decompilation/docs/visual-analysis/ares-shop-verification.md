# Ares Weapon Emporium verification

Status: the Ares event now traverses the field-exit selector and generic
mode-text prompt before opening a typed Qt shop controller. The presenter draws
the source service background, selected tab, fixed-width headings, item row,
one-pixel item selector, and source-specific Universal Gun Clips art/text. A
new-game wallet fallback displays the ROM's starting 20 nuyen. Purchase/sale
confirmation and mutation are not connected to the live session, restored-save
wallet data is not exposed by the save adapter, and actual sound playback is
unresolved; the Ares hill remains partial. The introduction and shop were
independently reviewed against the full longplay at 01:39:43-01:39:49.

## Source trace

- Entry: US ROM `0x1FA78`, field mode `FFFBF8=2`/event `FFFBF9=11`, selects
  mode resource `0x1220C0`, sets menu mode 1 and result -1, and selects text
  record `0x110DD2`. It calls `0x1F246` (whose `0x1F3C4` child handles the
  generic mode-text prompt), then dispatches `0x56AC0`. The converted field
  exit orchestrator now recognizes this exact mode/event pair, holds the shop
  dispatch until the prompt returns, and opens the typed Ares shop only for
  its default result. Alternate text results return to the field.
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
  shows the logo, text well, empty lower well, and full-panel redraw. The
  field-event QA exercises the same prompt-to-service route.
- Dialogue boundary: NPC prompt resolver `0x1F65A` matches mode 2/event 2 for
  the Stoker motel. Ares mode 2/event 11 follows `0x1FA78`; it must use its own
  event-selected mode-text data and must not borrow Stoker speaker assets.
- Selection: `0x56AC0` consumes `FFDD22` direction/confirm/return edges.
  `0x58C54` bounds the selection cursor, `0x58CDE` switches the two panels,
  `0x587DA` publishes the action selection, and `0x57C7A` redraws the selected
  action presentation. The menu has no periodic animation in this loop.
- Service tabs/headings: `0x58B5E` writes the 18x2 tab row from `0xCE17E`;
  `0x58BB6` adds the selected 2x2 overlay from `0xCE22E + cursor*8`. The
  heading table at `0x1AA8AF` contains 32 records with 20 visible characters
  and a separator byte. Extraction preserves their spaces because the source
  uses them to align the two centered title lines.
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
- Universal Gun Clips detail: the special offer type `0x45` takes the E branch
  in `0x586CA`. Its 3x2 art comes from `0xCE28E`, title from `0x1AACCE`, and
  description from `0x1AACE2`; the row carries quantity/price 25. The art uses
  palette line 1 from field palette setup `0xD29D6` plus `0x18AC56`, and its
  name words retain horizontal/vertical flips. The host importer emits a
  named 24x16 image and text rows. The runtime draws detail art at `(128,32)`,
  description at `(24,56)`, cost at `(192,112)`, title at `(104,144)`, and the
  stock icon in the first offer cell.
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

1. **Does it look correct?** The intro at 01:39:43 shows the Ares logo, green
   mode text, and empty lower well. The shop at 01:39:45 shows its title,
   two-line service label, stock row, service tabs, and right HUD. Native intro
   and category dumps were reviewed beside those independent longplay frames.
   The native QA fixture has Joshua's archetype-0 portrait and 20 nuyen, while
   the reference save shows another portrait and 7,923; those state values are
   not presented as a pixel match.
2. **Does appearance reflect behavior?** Mode 2/event 11 selects the Ares
   profile and event/result text. The default sentinel prompt has no A/B/C
   choice markers, waits 30 frames, and opens the shop only after returning to
   its caller. The six tab overlays follow the typed selected group, and the
   slim yellow selector follows the typed offer cursor. QA checks default
   routing, group selection, first offer selection, and input-gate timing.
3. **Are the visuals correct?** The independent 01:39:45 category and
   01:39:47 Universal Gun Clips frames were compared with `shop-category` and
   `shop-special-clips/actual.bmp`. The two-line category heading preserves
   its ROM spacing. The E offer's art shape, red/black palette, detail text,
   cost/quantity 25, tab, and item-row icon now match the longplay composition.
   The native selector QA requires a one-pixel 24x16 border and rejects extra
   selector-color pixels elsewhere in the 32x24 offer cell. The captures are
   conversion evidence; the longplay remains the visual authority.
4. **Animation?** The mode prompt waits 30 frames and has no periodic motion.
   The shop loop has no periodic animation; its static art changes only when
   input selects another category/item or opens a later transaction prompt.
   Fifteen consecutive 30 fps frames around 01:39:54 show the independent shop
   view stable between inputs.
5. **Sound?** The generic prompt makes no direct sound request. Shop movement
   and tab changes request event `0x25`; confirm/restart paths request `0x24`.
   The QA event sink confirms two `0x25` requests for cursor movement and two
   for category/item-panel movement. Actual sample identity, playback duration,
   and production playback are unresolved; a queued event ID is not an audio
   verification.
6. **Related screen content?** The independent sequence includes the mode
   prompt, black transition, shop category, selected clips/patch details,
   and a later transaction prompt. The service base owns the complete panel
   and right-side HUD backing; the presenter layers tabs, headings, offer art,
   selector, and funds once. The shop's portrait and wallet remain live-state
   fields over that backing.
7. **Multiple frames?** The interpretation uses longplay frames 5983 (intro),
   5985?5986 (shop/category), 5987 (clips), 5988?5989 (Stim/Trauma Patch),
   5992 (transaction prompt), and 5994 (updated funds), plus fifteen adjacent
   frames near 01:39:54. This confirms both stable presentation and input-driven
   changes across several states.
8. **Stale UI?** The mode prompt and shop each draw their own full-screen base.
   Shop QA saves the category, selected item, special clips detail, and return
   to field as separate `actual.bmp` frames. It verifies the shop replaces
   prompt/field pixels on entry and that Start return replaces the full shop,
   including the tabs, item art, detail text, and selector. No old shop controls
   remain over the field frame.

## Implemented controller slice

`ShopMenuController` wraps the existing typed field-service selection state and
returns purchase, sale, source-boundary, runner-change, and field-return
requests. It does not own a second inventory or wallet. The live presenter
consumes the named background, six extracted selected-tab images, fixed-width
heading records, existing named item images, and a dedicated E-offer art/text
asset. `ShopTransactionService` delegates to the existing typed
purchase/removal conversions, but the live shop currently records the
transaction request as an unresolved boundary instead of mutating session
inventory or funds. Debug build and five focused CTests pass; the field-shop QA
writes introduction, category, selected-item, moved-cursor, clips-detail, and
field-return `actual.bmp` checkpoints. The importer was run explicitly with
`shadowrun_graphics_extract --all-assets`; extraction is not part of CTest or
ordinary builds.
