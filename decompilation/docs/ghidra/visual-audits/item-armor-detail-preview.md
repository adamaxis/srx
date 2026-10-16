# Selected Armor Detail Preview

## Scope

This pass covers the armor information shown while an armor item is selected
in the field inventory. It does not close the broader `ITEMS-01` report about
weapon damage values, combat damage, shops, and the remaining item catalog.

## ROM path

`0x12A8C` draws the selected item and dispatches action codes `0x2C..0x32` to
`0x11352` after setting `FFF0B1` to the preview state. `0x11352` subtracts
`0x2B` from the selected action code to choose one of seven armor records at
`D4D50`, with a `0x30`-byte stride. It temporarily saves and replaces the
runner's `+0x76` equipped-armor ordinal and the selected cell's `+0x6E` phase,
renders the armor name and Wpns/Guns labels, draws the two modifier bars, then
restores both saved bytes.

The lower panel writes are at `C820`, `C922`, `C92C`, and `C9AC`, corresponding
to host positions `(128,128)`, `(136,144)`, `(176,144)`, and `(176,152)`. The
selected armor preview therefore replaces the lower armor summary for that
inventory frame. It does not add another pair of bars beside the selected-item
details near the top of the screen. When another item is selected, the lower
armor preview is cleared with the rest of the full-page redraw.

The bar source is the six-tile `CF884` table: `0x11352` multiplies each
D4D50 modifier by `0x0C` and sends six tiles from the shared table to both
Wpns and Guns. The native extractor writes `menu/field/armor-summary-bar-0`
through `-8` as 48x8 images. These are distinct from the seven-tile weapon
Damage rows at `CF84C` and Drain rows at `CF8F0`; reusing those detail assets
was a source mismatch even when the panel placement looked right.

The seven modifier pairs were read directly from the US ROM at `D4D50` through
`D4E70`, record fields `+0x25/+0x26`:

| Item code | Armor ordinal | Primary (`+0x25`) | Secondary (`+0x26`) |
|---:|---:|---:|---:|
| `0x2C` | 1 | 2 | 1 |
| `0x2D` | 2 | 1 | 2 |
| `0x2E` | 3 | 5 | 3 |
| `0x2F` | 4 | 4 | 3 |
| `0x30` | 5 | 3 | 2 |
| `0x31` | 6 | 6 | 4 |
| `0x32` | 7 | 8 | 6 |

## Code and behavior changed

`find_us_armor_definition_for_item_code()` now expresses the source
`item_code - 0x2B` mapping as a typed catalog lookup. The Qt inventory uses
that selected row to render the lower summary. The runner's equipped armor
remains authoritative on the combat/status page, while the preview uses the
selected item and leaves `+0x76` unchanged. The erroneous upper Wpns/Guns pair
was removed. The offline importer now extracts `CF884` directly, and both Qt
and NativePort consume its named six-tile summary images. The armor modifier
bytes are retained as typed native state; the native asset-state version is
now 5 and the reader still accepts versions 2 through 4.

## Nine-question visual review

| Question | Result and evidence |
|---|---|
| 1. Does it look correct? | **Yes, for placement and panel structure.** Independent section 06 frame `002560` shows selected Armor Clothing above a lower Armor Clothing/Wpns/Guns panel. It does not show a different selected and equipped armor. |
| 2. Does the visible look match behavioral state? | **Yes, internally.** Debug/Release QA checks Armor Vest while `+0x76` is zero and Armor Clothing while `+0x76` is still 1; the displayed bars follow the selected item and the typed equipped state remains unchanged. |
| 3. Are the visuals correct? | **Source-derived and internally checked.** All seven `D4D50` modifier pairs and item-code mappings are checked; the screenshot QA compares the lower bars to the selected modifiers' named `CF884` assets and asserts the old upper row is clear. The supplied gameplay frame supports layout, but exact bars for a differing selected/equipped pair remain unverified independently. |
| 4. Is it animated, and when? | **No.** `0x11352` redraws on selection; no animation counter or timed update is involved. Full-screen 30-frame holds for equipped and unequipped Vest remain static. |
| 5. Is there a sound? | **No preview sound call is traced.** Inventory cursor movement is silent in the field-action QA. The separate Equip confirmation still requests event `0x24`; this pass did not capture route-specific speaker output. |
| 6. Are the other parts of the screen correct? | **Yes, internally reviewed.** All 54 Debug frames were inspected as complete 320x224 screens, including weapon, discard, Clips, armor, gear, action menus, and returns. No unrelated stale controls or overlaps were found. |
| 7. Do multiple frames support the interpretation? | **Yes, with a reference limit.** Independent frames `002550`, `002560`, `002580`, and `002699` show the lower panel, selected Armor Clothing, the Medkit page without the armor panel, and the later inventory state. None contrasts different equipped and selected armor. |
| 8. Does the new UI replace the old UI cleanly? | **Yes.** The selected armor replaces the lower summary for that frame; selecting Medkit removes it. Entry, action child, equip, unequip, replacement, and parent frames are included in the whole-screen QA. |
| 9. Was the whole affected time window reviewed? | **Yes for the source states in this slice.** The 54-frame dump includes inventory/action transitions and 30-frame holds. The preview is static; the dump is a set of full-screen state checkpoints rather than a continuous video. |

## Internal verification

- `shadowrun_ghidra_armor_catalog` passes in Debug and Release and checks all
  seven source modifier pairs plus item-code-to-ordinal mapping.
- `shadowrun_ghidra_conversion_qt_field_action_qa` passes in Debug and Release.
  Its 54 named BMPs per configuration are all 320x224 and byte-identical
  across configurations. The added screen assertions check selected-row bar
  assets, no equipped armor, a selected armor that differs from the equipped
  one, the two-step replacement, and the cleared upper detail region.
- All 54 Debug full-screen frames were reviewed after the final placement
  and `CF884` asset-source changes. The armor screenshots show the six-tile
  source strips in the lower panel, no duplicate upper bars, and no stale
  armor summary when a Medkit is selected. Native Debug/Release graphics and
  NativePort tests also pass; the NativePort test checks the extracted strip
  dimensions and D4D50 modifier bytes.

These checks validate the Qt conversion against the ROM-derived table and
named assets; they are not an independent Genesis golden.

## Status

The selected-armor preview's wrong source row, wrong bar table, and wrong upper
placement are corrected in Qt and covered by Debug/Release QA. Independent frame `002560`
supports the lower-panel layout, and adjacent frames support clearing that
panel on a non-armor selection. The supplied capture does not show an
unequipped armor preview or a selected armor different from the equipped one,
so those exact visual states remain unverified against independent gameplay.
`ITEMS-01` remains open for weapon damage and combat values, shop presentation,
and this independent state comparison.
