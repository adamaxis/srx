# Item templates, weapon cadence, and numeric visuals

Date: 2026-10-10
Primary source: US Genesis ROM `input/shadowrun_genesis_us.bin`.
Cross-check: `export/disassembly.s` and the Ghidra C export.
Visual reference: `video analysis/long_gameplay_video.mp4`; no matching weapon
fire/menu-price capture was identified for these claims.

## Source-backed item template

The Lua catalog now describes each of the 50 source item rows by identity,
price, and item art, and keeps implemented item-specific behavior on the item
row. The weapon-specific fields include magazine capacity, damage-art row,
Power profile, round pattern, repeat behavior, attack-delay scale, and the
delay mode used by the source reload helper. Purchase and resale base values
are both retained; the dynamic runner adjustment remains the shop transaction's
runtime calculation.

The US ROM item table starts at `0xD4540` with a `0x30`-byte stride. For each
item, `+0x22` is the buy base, `+0x24` is the firearm magazine capacity,
`+0x25` selects the Damage art row, and `+0x26` selects the Power profile.
The sale path starts from integer half of the item's buy base before applying
the runner-dependent adjustment. Lua stores both bases; the purchase callback
reads `pricing.purchase_base`, and the sale callback reads
`pricing.resale_base` so the host does not halve the sale value a second time.

`0x38AC` distinguishes the player records from higher-type NPC records. For
player weapon actions, item codes 8, 9, and 10 send a `-3` meter delta; codes
1..7 and 11..12 send `-1`. The templates therefore call those three SMGs
`three_round_burst` with `rounds_per_action = 3`; the others use
`single_round` with `rounds_per_action = 1`. `0xACAE` checks held A and the
action delay gate, so `trigger_behavior = "repeat_while_held"` records that a
held button may request another action once the gate allows it. A single
source action still consumes only the template's one- or three-round amount.
NPC records follow the source's separate one-round decrement.

Weapon shot cadence is not one constant and is not an invented frames-per-shot
number. `0xB39A` reads the equipped action code at record `+0x57`: codes 11 and
12 select delay mode 0 (normal scale), while codes 1..10 select mode 1 (half
scale). `0xABFC` computes the underlying delay from actor attributes and
capability flags through `0xD7EA`, scales it, and clamps it to at least 15.
Consequently weapon codes 1..10 use the shorter half-scaled delay and
Allegiance (11) and Roomsweeper (12) use the longer normal-scaled delay, with
the final frame count still depending on the runner. This item-to-mode mapping
now comes from Lua and is passed into the live C042 attack-pose path.

The source reload helper at `0x14226` also selects a delay mode from item code
(double scale for 5, 11, and 12; normal otherwise). Its parent `0xACAE` then
performs a normal delay update after a successful reload dispatch. The Lua
template retains the helper's input as `reload_delay_scale`; the final live
gate follows the parent's later write.

## Named item art and number rendering

Each Lua item row provides an icon resource key and a detail resource key.
The field renderer now consumes these keys directly for both icon palettes and
detail pictures instead of reconstructing asset families from item-code
ranges. The shop template test checks all 50 keys; the field shop QA boots the
runtime with the complete icon/detail set, which fails if any named resource
is missing.

`build_numeric_visual` converts an integer to a left-to-right glyph run,
including optional thousands separators. The Qt `render_numeric_visual`
helper clips those glyphs from whichever extracted font atlas the current menu
uses, applies the same green ink as the command/shop text when requested, and
supports left/right alignment. Shop quantity, price, wallet, and sale values
now share it; clips, attribute values, magic force, and notebook page counts
also use it with their own menu font/palette. The old ROM glyph-index adapter
remains available for source-order conversions.

## Verification record

- The item catalog test compares all 50 buy/resale bases to the decoded item
  rows and checks each icon/detail key. It checks all 12 weapon round patterns,
  held-trigger behavior, actor-derived cadence, and the per-item attack/reload
  delay mode.
- The ammo service reads `rounds_per_action` through a callback. The live field
  runtime supplies that callback from the equipped Lua weapon template; the
  tests cover one-round actions, three-round actions, and unresolved templates.
- The heading-delay conversion receives an explicit source delay mode rather
  than inferring weapon speed from an unrelated code in the C++ helper. The
  live player action path resolves the mode from its Lua template.
- Numeric-renderer tests check glyph grouping, tint, transparent margins, and
  right alignment. The complete Debug and Release CTest suites each pass
  98/98. Native-size screen captures were reviewed for shop, clips, attributes,
  magic, notebook, and player-action states, including panel redraws, item art,
  prices, selectors, HUD, and sale/purchase overlays; no stale prior-page
  controls remain in those captured states. The corresponding Debug and
  Release BMPs are byte-identical across 447 compared images.
- Numeric values are static during these screens and introduce no animation
  or sound requests. Existing menu selection/transaction audio paths are
  unchanged. Independent Genesis screenshots for each weapon firing mode,
  attack cooldown, and shop numeric layout are still unverified, so this is a
  source-backed native implementation with internal screenshots, not a claim
  of independent visual equivalence.
