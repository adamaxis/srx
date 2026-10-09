# Ares Weapon Emporium verification

Status: the Ares event now traverses the field-exit selector and generic
mode-text prompt before opening a typed Qt shop controller. The presenter draws
the source service background, selected tab, fixed-width headings, item row,
one-pixel item selector, and source-specific Universal Gun Clips art/text. The
new-game wallet uses the ROM's starting 20 nuyen. Purchase, E-coded clip,
confirmation, sale/refund, and protected-item paths now mutate the active
runner and new-game wallet through typed state. The restored-save adapter still
does not expose wallet funds. Events `0x24` and `0x25` now reach extracted
one-shot files through the Qt playback service; their source event identity
and file durations are recorded below. Shared audio QA decodes and starts
events `0x24`, `0x25`, and `0x4B` on the default output and checks duration and
stop behavior; Windows loopback for the exact shop route remains unrecorded.
The Ares hill remains partial. The introduction and shop were independently
reviewed against the longplay sequence around frames 5983-5994. Separate
physical-entry QA starts at New Game and uses ordinary Right/Up input to
contact scene-2 record 92 (`0xCB`/event 11), waits through the generic prompt,
enters the shop with A, and returns with Start. Debug/Release captures cover
the field approach, fade, prompt, category page, and field return; see
[`../visual-audits/ares-physical-entry.md`](../visual-audits/ares-physical-entry.md).

## Source trace

- Entry: US ROM `0x1FA78`, field mode `FFFBF8=2`/event `FFFBF9=11`, selects
  mode resource `0x1220C0`, sets menu mode 1 and result -1, and selects text
  record `0x110DD2`. It calls `0x1F246` (whose `0x1F3C4` child handles the
  generic mode-text prompt), then dispatches `0x56AC0`. The converted field
  exit orchestrator now recognizes this exact mode/event pair, holds the shop
  dispatch until the prompt returns, and opens the typed Ares shop only for
  its default result. Alternate text results return to the field.
- Physical collision: scene-2 record 92 at `0x173142` contains bounds
  `0x4C0..0x4DF` by `0x220..0x23F` and value `0xCB`. From the New Game spawn,
  ordinary Right then Up contacts the expanded rectangle at `(1231,582)` and
  publishes raw/projected `0xFFCB` with event 11. Physical-entry QA checks
  this state without injecting collision/event tuples and captures the fade,
  prompt, shop entry, and clean field return.
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
- Audio: ROM `0x56C72`, `0x56C8E`, `0x56CAA`, and `0x56CC8` load `D0=0x25`
  before calling `0x530DE`; the enabled queue path passes that value to
  `0x52F3C`. The Z80 offline importer exposes 94 zero-based table-2 streams,
  so byte `0x25` selects decimal entry 37, extracted as
  `audio.effects.resource.037`. The Qt alias test checks both the selected
  alias and that the playback service forwards it to the asset resolver.
- Service tabs/headings: `0x58B5E` writes the 18x2 tab row from `0xCE17E`;
  `0x58BB6` adds the selected 2x2 overlay from `0xCE22E + cursor*8`. The
  heading table at `0x1AA8AF` contains 32 records with 20 visible characters
  and a separator byte. Extraction preserves their spaces because the source
  uses them to align the two centered title lines.
- Title: `0x56AC0` indexes the 12-entry pointer table at `0x1AAB50` with
  `FFF0B6`. Ares uses entry 1, whose record at `0x1AAB92` stores destination
  cell `0xC110` and text `Ares Weapon Emporium`. The offline extractor emits
  all 12 titles as keyed Lua text in `dialogue/script-text.lua`; numeric
  destination words remain as extracted routing data in
  `menu/field/service-routes.tsv`.
  All other field menu copy now comes from the shared Lua text table. The Qt
  runtime does not follow those ROM pointers.
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
  named 24x16 image and Lua text entries. The runtime draws detail art at `(128,32)`,
  description at `(24,56)`, cost at `(192,112)`, title at `(104,144)`, and the
  stock icon in the first offer cell. All 12 service titles, 32 fixed-width
  service headings, and seven shop transaction strings now live in
  `dialogue/script-text.lua`;
  `gameplay/scripts/shop.lua` associates the Ares title, six category heading
  pairs, stock, special clips copy, and transaction-message keys. The shared
  `items.lua` catalog maps the 50 source item IDs and all 32 detail rows to
  ROM-generated text; the native controller still owns item codes and state.
- Mode-text paragraphs selected by `0x1FA78` now come from ROM records at
  `0x118934`, `0x11898B`, `0x1189D0`, `0x1189F2`, `0x118A1E`, `0x118A46`,
  `0x118A6C`, and `0x118AA4` in `dialogue/script-text.lua`. The typed native
  event/result selector chooses introduction, package handoff, or payment
  text; the generic dialogue panel and 30-frame wait remain shared.
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
   slim yellow selector follows the typed offer cursor. Transaction state
   controls the default-No cursor, Yes movement, message wait, funds, ordinary
   slot writes, special-E count, sale refund, and protected-item response. QA
   checks these values against the active runner and wallet before/after each
   input; it does not infer selection from pixels alone.
3. **Are the visuals correct?** The independent 01:39:45 category and
   01:39:47 Universal Gun Clips frames were compared with `shop-category` and
   `shop-special-clips/actual.bmp`. The two-line category heading preserves
   its ROM spacing. The E offer's art shape, red/black palette, detail text,
   cost/quantity 25, tab, and item-row icon now match the longplay composition.
   The Stim Patch detail and confirmation captures were compared with
   independent frames 5988 and 5992. The confirmation keeps the selected art,
   title, row, and HUD, clears the old description, defaults to No, and does
   not leave price text under the prompt. The sale quote appears on its
   confirmation line; it no longer collides with the idle item description.
   Selector QA requires a one-pixel 24x16 border and rejects extra selector
   color elsewhere in the 32x24 offer cell. Native captures are conversion
   evidence; the longplay remains the visual authority.
4. **Animation?** The mode prompt waits 30 frames and has no periodic motion.
   The shop loop and confirmation are static between controller events. The
   same selected item art persists through the prompt; input moves the Yes/No
   cursor or ends the transaction. No extra animation was observed in the
   adjacent longplay frames.
5. **Sound?** The generic prompt makes no direct sound request. The ROM shop
   loop at `0x56C72..0x56CCC` requests `0x25` for selection movement; that
   byte is table-2 index 37 and maps to `audio.effects.resource.037`. Its FLAC
   is 2.134 seconds including the extractor's two-second release tail, and
    the Qt service sends it through `QMediaPlayer`/`QAudioOutput`.
    Confirm/cancel paths request
   `0x24`, mapped to `audio.menu.command-024` (2.484 seconds). A
   pre-emphasized waveform comparison against longplay audio from seconds
   5980-6000 found repeated strong matches for resource 037 at 5985.289,
    5987.022, and 5993.456. Debug/Release QA verifies FLAC decoding,
    `PlayingState` on the default output, duration, and stop behavior for event
    `0x25` (along with `0x24` and `0x4B`). It does not record Windows loopback
    for the exact shop action.
6. **Related screen content?** The independent sequence includes the mode
   prompt, black transition, shop category, selected clips/patch details,
   and a later transaction prompt. QA also inspects the insufficient-funds,
   sale/refund, and protected-Totem screens. The service base owns the full
   frame and HUD backing; the presenter redraws the selected tab, item detail,
   selector, transaction text, and wallet from current state. Modal text
   replaces the item detail in the shared text well while leaving the intended
   item art and lower row in place.
7. **Multiple frames?** The interpretation uses longplay frames 5983 (intro),
   5986 (shop/category), 5987 (clips), 5988-5989 (Stim/Trauma Patch), 5992
   (Stim Patch confirmation), and 5994 (updated funds), plus fifteen adjacent
   frames near 01:39:54. The native QA produces separate `actual.bmp` files for
   category, item, confirmation-No, confirmation-Yes, purchase, sale, protected
   sale, and return. Independent longplay frames anchor the visual reading;
   native frames verify the typed state transitions.
8. **Stale UI?** The mode prompt and shop each draw their own full-screen base.
   Shop QA saves category, selected item, purchase modal, sale modal, failure
   message, protected-item message, and field return as separate `actual.bmp`
   frames. It checks that modal redraw removes stale description/price text,
   preserves the intended item art/title/row, and that Start return replaces
   the whole service page, including tabs, item art, details, and selector.
9. **Do new UI elements cover old UI that should be gone?** The full-screen
   route and all 21 shop transaction captures were reopened at 320x224. The
   category page has only its active headings, tabs, offer row, wallet, and
   right HUD; confirmation and feedback states clear old detail text while
   retaining only the intended item art, row, and HUD. Field return redraws
   the field page and leaves no shop tabs, selector, or text behind. Debug and
   Release captures match by SHA-256; all 14 route captures also match the
   pre-migration hashes.

## Implemented controller slice

`ShopMenuController` wraps the existing typed field-service selection state and
returns purchase, sale, source-boundary, runner-change, and field-return
requests. It does not own a second inventory or wallet. The live presenter
consumes the named background, six extracted selected-tab images, fixed-width
heading records, existing named item images, and a dedicated E-offer art/text
asset. `shop.lua` binds the common prompt, transaction feedback, purchase/sale
headings, and special clips text; `items.lua` maps the 50 item records and 32
detail strings generated from the ROM pointer table. `ShopTransactionService` delegates to the
existing typed purchase/removal conversions, and the field session applies
successful mutations to its active runner and new-game wallet. Debug QA writes
introduction, category, selected-item, purchase failure/cancel/accept, Stim
Patch confirmation, E-coded clips, sale/refund, protected-sale, and return
`actual.bmp` checkpoints. The importer was run explicitly with
`shadowrun_graphics_extract --all-assets`; extraction is not part of CTest or
ordinary builds. Debug and Release also pass script runtime, field-menu text
catalog, physical Ares entry, and full shop transaction QA. Loaded-save wallet
binding and Windows loopback capture for the shop action are still open.
