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
reviewed at source timestamps 5983-5994 seconds (01:39:43-01:39:54). In the
30fps section dump, source timestamp 5983 seconds is local frame `004890`;
frame filenames are section-local and are not source-second numbers. Separate
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
  without entering `0x56AC0`. `field_mode.lua` receives the read-only native
  mode/event/result context and selects all three text keys plus the Ares logo
  asset key; the text itself remains in the ROM-imported Lua catalog. This
  selection does not assign the prompt Stoker's speaker or portrait.
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
  uses them to align the two centered title lines. `shop.lua` returns the
  explicit keyed text binding beside each stable service/action ID; the Lua
  runtime validates that ordered binding before the native presenter resolves
  the corresponding ROM text.
- Title: `0x56AC0` indexes the 12-entry pointer table at `0x1AAB50` with
  `FFF0B6`. Ares uses entry 1, whose record at `0x1AAB92` stores destination
  cell `0xC110` and text `Ares Weapon Emporium`. The offline extractor emits
  all 12 titles as keyed Lua text in `dialogue/script-text.lua`;
  `gameplay/scripts/shop.lua` declares their stable IDs in the same pointer-
  table order. Numeric destination words remain as extracted routing data in
  `menu/field/service-routes.tsv`.
  All other field menu copy now comes from the shared Lua text table. Each
  service ID returned by `Shop.services()` is paired there with its explicit
  `shop.service.option.*` text key; the C++ menu runtime no longer has to
  manufacture that key. The Qt runtime does not follow the ROM pointers.
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
  named 24x16 image and Lua text entries. The runtime draws detail art at
  `(128,32)`, description at `(24,56)`, and the amount immediately left of the
  currency glyph embedded in that description (currently `(184,112)` for 25).
  The title is at `(104,144)`, and the stock icon is in the first offer cell.
  All 12 service titles, 32 fixed-width
  service headings, and shop transaction strings now live in
  `dialogue/script-text.lua`; `gameplay/scripts/shop.lua` declares their stable
  IDs and associates the Ares title, six category heading
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
  and `0x1AAD8F`. Direct US-ROM reads confirm zero-based action row types
  `01,02,06,04,05,0D,0E`, `45,0F,10,11`, and `2E,2D` respectively. `0xFF` in
  byte 0 ends each offer list. `type + 1` selects the one-based item record.
  Those rows are represented as Lua-defined typed records. `0x586CA` sends
  selected ordinary offers through `0x130C0`, whose source detail class
  distinguishes firearm `ammo` from item `uses`. The single `items.lua`
  catalog defines all 50 item records. Each of the twelve firearm records
  owns its source-derived magazine capacity, Damage profile, and Power
  profile; there is no parallel weapons list. The Ares offer order resolves to
  Model 101T, American L36, Predator, Warhawk, and Max Power, with capacities
  12, 11, 15, 6, and 10, followed by Scatter and Concussion grenades with six
  uses each. Grenades are not assigned firearm magazines. The right HUD count
  remains the active runner's loaded rounds.
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
   `shop-special-clips/actual.bmp`. The quantity sits immediately before the
   source currency glyph, so the final `5` no longer overlaps it. The
   two-line category heading preserves
   its ROM spacing. The E offer's art shape, red/black palette, detail text,
   cost/quantity 25, tab, and item-row icon now match the longplay composition.
   The Stim Patch detail and confirmation captures were compared with
   independent frames 5988 and 5992. The confirmation keeps the selected art,
   title, row, and HUD, clears the old description, defaults to No, and does
   not leave price text under the prompt. The sale quote appears on its
   confirmation line; it no longer collides with the idle item description.
   The five Ares firearm offers resolve through their zero-based action rows
   to named Lua item templates and display capacities 12, 11, 15, 6, and 10
   beside `ammo` at `(104,56)`. Scatter and Concussion are item-use records
   and display `uses 6`; they do not show magazine capacities. The number sits
   between the label at `(72,56)` and art beginning at `(128,32)` without
   obscuring the label, art, selector, or HUD. No supplied longplay frame
   selects these details, so exact Genesis spacing is not independently
   verified.
   Selector QA requires a one-pixel 24x16 border and rejects extra selector
   color elsewhere in the 32x24 offer cell. Native captures are conversion
   evidence; the longplay remains the visual authority.
4. **Animation?** The mode prompt waits 30 frames and has no periodic motion.
   The shop loop and confirmation are static between controller events. The
   same selected item art persists through the prompt; input moves the Yes/No
   cursor or ends the transaction. All seven Ares stock detail rows and the
   selected number remain pixel-identical through a 30-frame native idle
   hold, with no repeated sound request. No extra animation was observed in
   the adjacent independent longplay frames.
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
   `0x25` (along with `0x24` and `0x4B`). The seven ordinary stock-row
   movements request seven `0x25` events; an idle hold requests none. It does
   not record Windows loopback for the exact shop action.
6. **Related screen content?** The independent sequence includes the mode
   prompt, black transition, shop category, selected clips/patch details,
   and a later transaction prompt. QA also inspects the insufficient-funds,
   sale/refund, and protected-Totem screens. The service base owns the full
   frame and HUD backing; the presenter redraws the selected tab, item detail,
   selector, transaction text, and wallet from current state. Modal text
   replaces the item detail in the shared text well while leaving the intended
   item art and lower row in place. The first firearm capacity is tested with
   the active runner's loaded count set to 5, so the 12-round template and
   sidebar's loaded ammunition remain distinct. The two grenade rows instead
   show their Lua item-use count. The full-screen review checks the right HUD,
   wallet, all offer cells, service tabs, description, price, and item art
   together; no unrelated region changed.
7. **Multiple frames?** The interpretation uses longplay frames 5983 (intro),
   5986 (shop/category), 5987 (clips), 5988-5989 (Stim/Trauma Patch), 5992
   (Stim Patch confirmation), and 5994 (updated funds), plus fifteen adjacent
   frames near 01:39:54. In `section_25_source_5820-6120`, those source-second
   positions map to local 30fps files `frame_004890`, `004950`, `005010`,
   `005040`, `005160`, and `005220`, respectively. The local file
   `frame_005983` is a later street-map frame; it is not the Ares intro.
   The longplay does not show a selected firearm or grenade detail page. Native
   QA writes five full-screen template-capacity states, two grenade-use states,
   and a 30-frame static hold. The complete current route has 32 named
   full-screen `actual.bmp` checkpoints; every Debug image was inspected at
   320x224, and all 32 relative paths have matching Release SHA-256 hashes.
   These verify source values and redraw behavior, not independent visual
   equivalence. Independent frames still anchor category, clips, patch,
   confirmation, and balance screens.
8. **Stale UI?** The mode prompt and shop each draw their own full-screen base.
   Shop QA saves category, selected item, purchase modal, sale modal, failure
   message, protected-item message, and field return as separate `actual.bmp`
   frames. It checks that modal redraw removes stale description, price, and
   selected-item numeric text, preserves the intended item art/title/row,
   and that Start return replaces the whole service page, including tabs, item
   art, details, and selector.
9. **Do new UI elements cover old UI that should be gone?** The full-screen
   route and all 32 current shop checkpoints were reopened at 320x224. The
   category page has only its active headings, tabs, offer row, wallet, and
   right HUD; confirmation and feedback states clear old detail text while
   retaining only the intended item art, row, and HUD. Template capacity and
   item uses fit the existing detail line without covering another control.
   The clips quantity and currency glyph occupy adjacent eight-pixel cells.
   Field return redraws the field page and leaves no shop tabs, selector, or
   text behind. Debug and Release captures match by SHA-256.

## Implemented controller slice

`ShopMenuController` wraps the existing typed field-service selection state and
returns purchase, sale, source-boundary, runner-change, and field-return
requests. It does not own a second inventory or wallet. The live presenter
consumes the named background, six extracted selected-tab images, fixed-width
heading records, existing named item images, and a dedicated E-offer art/text
asset. `shop.lua` binds the common prompt, transaction feedback, service
labels, action headings, purchase/sale headings, and special clips text; its
menu functions return explicit ordered text keys with the stable IDs.
`items.lua` contains all 50 item records; each name and category resolves
through a stable key in the extracted text catalog. Its 32 detail strings
remain keyed to the ROM pointer table. `ShopTransactionService` delegates to the
existing typed purchase/removal conversions, and the field session applies
successful mutations to its active runner and new-game wallet. Debug QA writes
introduction, category, selected-item, purchase failure/cancel/accept, Stim
Patch confirmation, E-coded clips, sale/refund, protected-sale, and return
`actual.bmp` checkpoints. The importer was run explicitly with
`shadowrun_graphics_extract --all-assets`; extraction is not part of CTest or
ordinary builds. Debug and Release also pass script runtime, field-menu text
catalog, physical Ares entry, and full shop transaction QA. Loaded-save wallet
binding and Windows loopback capture for the shop action are still open.

## Lua menu-label binding follow-up (2026-10-09)

`Shop.services()` and `Shop.action_headings()` now return the ordered stable
IDs and their explicit label keys together. The generic Lua runtime checks the
key list against the row count; `FieldSceneRuntime` continues to check the ROM
row order and resolves the keys through the imported text resource. This
removes the implicit C++ `profile.option.<id>` label construction for these
two shop menus. No menu layout, selection, transaction, asset, or audio code
changed.

The full-screen review reconfirmed the Ares title and category headings, the
slim yellow item selector, selected item art/details, default-No and moved-Yes
confirmation state, purchase/sale feedback, protected-item page, and field
return. All 21 named Debug BMP checkpoints were inspected individually at
320x224; matching Release hashes establish the same pixels in both builds.
The modal pages replace old price
and description text while retaining only the intended item art, offer row,
and HUD; field return removes the entire shop panel. The shop loop has no timed
idle animation; item/tab selection changes only on controller events. ROM
movement/confirmation sounds and their extracted durations remain as traced
above; this Lua mapping does not add or change a sound request. Independent
longplay frames support the Ares title, category, item, confirmation, and
updated-funds sequence, but do not supply a second complete menu traversal.
This is a text-binding/runtime consistency check, not a new Genesis fidelity
claim.

## Universal Gun Clips price spacing follow-up (2026-10-10)

The independent 30-fps longplay frames at source time 01:39:47 show `25` in
the two cells immediately left of the currency glyph on the final `Cost per
Clip` line. The ROM-derived Lua text at `0x1AACE2` carries that glyph in the
description itself: after drawing the description from `(24,56)`, its final
line places the marker at `(200,112)`. The previous hard-coded quantity origin
`(192,112)` put the final digit in the marker cell. The presenter now finds
that marker in the Lua-fed text, computes its eight-pixel row and column, and
right-aligns the quantity immediately before it. The amount and marker no
longer share pixels.

The Debug and Release Ares shop QA was rerun after this change. All 21 current
full-screen checkpoints were inspected at 320x224, including the introduction,
category, item focus, clips/patch details, purchase and sale prompts, feedback,
and field returns. Their Debug/Release BMP hashes match. A leftover
`shop-sale-quote` BMP dated 2026-10-04 was not produced by the current test and
was excluded from this review. The independent captures at 01:39:47 and
01:39:48 show the clips and patch price rows; no periodic shop animation or
additional sound is associated with this layout-only fix. The shop remains
partial: unaffordable-feedback duration and the reported category activation
sequence are still open.

## Equipped weapon sale cleanup follow-up (2026-10-10)

ROM `0x56DC4` compares the selected sale row against the active runner's
equipped slot at `+0x56` and sets `FFF098=-1` on a match before calling
`0x1275C`. The cleanup routine then writes the unarmed sentinel to `+0x56`,
clears the active item marker at `+0x57`, and clears loaded rounds at `+0x4E`;
it also clears the sold inventory code/value pair at `+0x66+slot` and
`+0x6E+slot`. The typed sale adapter already cleared the equipped slot and
item marker but had left `ammo_rounds` stale. It now clears all three fields
for that source branch and refreshes the field HUD.

The Ares QA sells Joshua's equipped Samurai weapon after dismissing the
protected-Totem response. It drives the real sell cursor and default-No
confirmation, captures confirmation, result, and settled full-screen frames,
checks the refund and inventory row, then asserts the active slot/item/rounds
and HUD counter are cleared.

Visual review for this slice:

1. **Does it look correct?** The post-sale screen shows the unarmed/Spurs HUD
   icon with `00`; the selected inventory row and its sale detail are empty.
2. **Does the visual match behavior?** The live runner and derived HUD
   projection both report no equipped slot/item and zero displayed rounds;
   the inventory code/value pair is cleared.
3. **Are the visuals clean?** The Ares title, HUD frame, and lower controls
   remain. The old confirmation/detail text is absent, and the right HUD
   region changes from the equipped-gun presentation.
4. **Animation?** ROM `0x56AC0` has no periodic shop animation. A 30-frame
   post-sale hold is pixel-identical to the immediate result.
5. **Sound?** The same confirmation route requests `0x24` on opening/closing
   and `0x25` for moving the selection; the QA checks that request sequence.
   Extracted event `0x24` lasts 2.484 seconds including its release tail.
   Clearing the sold slot adds no separate sound. Windows route loopback is
   still unrecorded.
6. **Related screen elements?** The full 320x224 captures include the shared
   shop shell, inventory row, wallet, and right HUD, so stale text and the old
   weapon counter are reviewed together with the sale result.
7. **Multiple frames?** The native captures cover confirmation, immediate
   result, and 30-frame settled state. Longplay frames at source 01:39:47,
   01:39:48, 01:39:50, and 01:39:52 show the clips/patch shop sequence, while
   01:39:54 shows a 204-nuyen balance change; they do not show an equipped
   weapon being sold, so no independent post-sale visual comparison is claimed.
8. **Old UI underneath?** The sale confirmation is replaced by the empty-row
   detail well. The full-screen result retains only the Ares page and its
   shared controls; the 30-frame equality check catches leftover or animated
   pixels.

The three new Debug/Release full-screen checkpoints match by SHA-256 and were
inspected at 320x224. General weapon stock detail and Spurs/unarmed combat
presentation remain open under `SHOP-03` and `COMBAT-STATE-01`.

## Semantic weapon templates and shop detail follow-up (2026-10-10)

The ROM's D4540 item records 1..12 are the firearm templates: `+0x24`
supplies the magazine capacity used by the ammo service and weapon-switch path, `+0x25`
selects a Damage display profile, and `+0x26` selects a Power display profile.
Direct US-ROM reads match those fields to the twelve nested firearm templates
in the single `items.lua` catalog. The next three item codes are grenades, not
firearms; their source action-detail label is `uses`, so they carry a semantic
item `uses` count and never receive a magazine capacity. The catalog also
binds all 50 names, categories, base costs, and detail-art flags, plus the
seven D4D50 armor templates. Item-detail conversion now requires the typed
item catalog instead of a separate synthetic detail-value array.

The presenter resolves the selected item to its Lua content: firearm templates
draw `ammo` plus `magazine_capacity`, while grenade records draw `uses` plus
their item-use count. The right HUD remains the live runner's loaded-round
state. QA deliberately sets that HUD count to 5 while selecting the 12-round
Model 101T, then verifies all five Ares firearm templates, both grenade-use
records, detail labels, selector position, seven movement sound requests, and
a 30-frame static hold with no additional sound.

Debug and Release CTest pass the script runtime, Ares collision/shop route,
shop transaction, and shop menu runtime tests (4/4 in each configuration).
The three workbench item-detail conversion tests also pass in Debug and
Release; both shop-action and inventory detail now resolve through typed
source item records without a parallel numeric detail table.
The test writes 32 whole-screen `actual.bmp` checkpoints per configuration;
all 32 Debug/Release paths match by SHA-256, and all 32 Debug images were
reviewed at 320x224, including the eight selected-capacity/idle states, modal
redraws, equipped-sale HUD, both field returns, and the intro/category/other
item pages. No stale numeric detail remains on modal screens or the returned
field. Selected firearm and grenade detail pages are absent from the longplay,
so their placement is supported by ROM data and renderer QA but remains
unverified against Genesis pixels. The route's exact Windows output loopback remains
unrecorded; event 0x25 duration and default-output playback are covered by
shared audio QA.
