# Council Island Medicine Lodge Field Prompt

Date: 2026-10-20  
Status: partial; Debug and Release route, Lua shop-state, adapter, spell/item/
focus purchases, and both physical-entry CTests pass. Independent Genesis
comparison remains open.

## ROM route

Scene 4 collision record 88 is at `0x15BDA6`, bounds `0x0500..0x051F` by
`0x0040..0x005F`, raw collision `0x00C3`, event 3. Event-table byte 3 at
`0x15B95E` is `0x0D`, but field mode service at `0x1FA78` intercepts mode 4,
event 3 at `0x1FCF2`; this is a same-map conversation, not a transition into
scene 13. The source handler selects dialogue root `0x111876` or `0x111892`
from FFFC1E bit 4. Both roots offer shaman hiring and store browsing. The
third choice is the ordinary farewell at the first root and the "most wise
one" option at the second root.

The hire command `0xF0` reaches `0x24C34`, sets FFFC2A bit 4, and displays the
Walking Bear referral rooted at `0x11A8FD`. Store command `0x1E` reaches
`0x21982`, sets FFF112, and dispatches shared field service `0x56AC0` with
mode 3. The service entry writes `FFF0B6=3` and `FFF0B7=0`. Command `0x1F`
reaches `0x21992`, stores the eight-word scene-entry profile at `0x1668A` in
`FFE114`, and returns the `0x1118DA` text directing Joshua to find the wise one
outside the door to his left. The field runtime queues this typed placement
for the next full scene entry. The signed alternate-result dispatch at `0x1FD04` selects root
`0x1118E2` for a negative result and `0x1118EA` for a positive result. The
ROM pointers at those roots resolve to package handoff text `0x11AA1E` and
package payment text `0x11AA98`, respectively; both pages return to the same
field map. These are ordinary conversation branches, with no jail, fine, or
empty positive result.

## Native behavior

`council_medicine_lodge.lua` owns the default/quest-dependent choice list,
typed runner-information flag, dialogue result IDs, and explicit service and
profile handoffs. The reusable linked-location resolver supplies the
scene/event route and same-map return anchor. `shop.lua` owns mode-3 menu state,
category navigation, and selection events; the C++ presenter consumes typed
Lua snapshots and forwards item actions to the shared transaction service.
Spell progression uses the selected runner's live `FFF9FA` row, initialized
from ROM rows at `0x166AD8` and selected by runner field `+0xA0`. Physical QA
buys the first spell, first magic item, and first focus through the menu,
including Yes confirmations. It checks force 1 -> 2, funds 10000 -> 8000 ->
7500 -> 3500, and both inventory records. It then round-trips the purchased
spell, inventory records, and wallet through the host save snapshot, restoring
a lowered spell copy followed by the saved state.

## Mode-3 service data trace

The result-zero page uses four category rows at `0x1AAAA7 + cursor * 0x2A`
(`0x58BB6`), and `0x58C54` clamps its primary cursor to 0..3. The row keys
bind to Magic Spells, Magic Items, Magic Foci, and Sell Item From / Inventory.

- Cursor 0 selects `0x1AAF40 + (FFF0B6 - 3) * 0x20`. Its mode-three block
  contains action codes `03,04,0C,05,0B`, metadata `06`, and base cost
  `0x03E8` (1000). The `0x57D7E` display path uses each offer byte directly
  as the `D4EA0` spell-row index, yielding Invisibility, Sleep, Confusion,
  Stink, and Heal Wounds. `0x56E5A` applies a confirmed purchase to the live
  progression row; this is separate from ordinary inventory purchase.
- Cursor 1 selects three-byte records from
  `0x1AAEF8 + (FFF0B6 - 3) * 0x18`. The mode-three block contains
  `(12,0A,00)` and `(26,04,14)` before the `FF` sentinel.
- Cursor 2 selects three-byte records from
  `0x1AAEB0 + (FFF0B6 - 3) * 0x18`. The mode-three block contains
  `(1B,02,00)`, `(1B,03,14)`, `(1C,02,00)`, and `(1C,03,14)` before the
  sentinel.
- Cursor 3 uses the active runner's inventory and the shared removal path.

`shop.lua` preserves these rows as separate progression and item records.
The parser and Lua state tests validate the result-zero service definition,
ROM cursor bounds, category movement, spell handoff, and item selection. The
physical route captures each category, first spell/item/focus details, and both
item and focus Yes-confirmation screens before checking the typed inventory
records and wallet.

## Verification

The Debug and Release native runtime tests resolve the scene-4/event-3 entity
link, check the default labels, and verify the hire choice sets the typed
runner-information bit and yields the extracted referral text. The Lua suite
covers the bit-4 choice switch, store result, farewell, package handoff, and
package payment branches. The Debug and Release physical-entry CTests start
south of record 88, use ordinary Up followed by the source B option, verify
event 3 and same-map field mode 4, then write full 320x224 offer, category,
spell/item/focus detail, transaction confirmation, purchase, field-return,
and save-restored captures. The purchased and restored captures match between
Debug and Release by SHA-256. The whole images, including the bottom edge,
were inspected. The map-only QA checkpoint has mode zero, so the save
round-trip probe normalizes it to scene 4's mode one before restoration. These
BMPs are internal regression artifacts; no independent Genesis service-screen
frame has been identified, so they do not establish visual fidelity by
themselves.

The separate quest-gated physical-entry CTest selects the wise-one option,
confirms the left-door direction text, and verifies that the next full scene
entry consumes profile `0x1668A`, placing Joshua at `(0x5B00, 0x0880)` while
remaining on scene 4.

## Remaining source boundaries

- Independent Genesis visual verification remains unavailable.
