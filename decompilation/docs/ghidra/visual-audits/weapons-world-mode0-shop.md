# Weapon's World mode-0 shop review

**Status:** Partial. Scene-5 event 5 now reaches the shared Lua field-shop
presenter after its mode-text prompt. The mode-0 catalog and first offer are
covered in Debug and Release. Accessory actions, purchases, audio, and an
independent Genesis screen comparison remain open.

## ROM trace and catalog

Scene-5 event 5 is intercepted by `0x1FA78` and selects the `weapons_world`
field service. The shared entry prompt is handled by the Lua field-mode
selector. Accepting the prompt calls the source field service loop at
`0x56AC0`; its negative-result action path at `0x56DC4` chooses a table by
category cursor and `FFF0B6` shop mode.

For Weapon's World, the catalog definition uses mode 0 and menu result -1.
The first three category tables contain these three-byte records:

| Cursor | ROM row | Label | Records |
| --- | --- | --- | --- |
| 0 | `0x1AAC86` | Weapons | `03/00/0F`, `04/00/0B`, `05/00/0E`, `06/00/0C`, `0B/00/0F`, `0E/06/00` |
| 1 | `0x1AADBF` | Armor | `2B/00/00`, `2C/00/00` |
| 2 | `0x1AAD77` | Equipment | `45/00/28`, `0F/06/14`, `11/04/00`, `10/04/00` |

The headings come from `0x1AA8AF + cursor * 0x2A`. `shop.lua` preserves
these rows as named Lua groups. C++ selects the shared field-store controller
from the Lua definition's negative result and six-category shape; the service
identifier selects the catalog and heading text.

## Route and captures

`shadowrun_ghidra_conversion_qt_field_scene5_store_service_entry_qa` follows
Scene-5 event 5, accepts the ROM-backed prompt, and checks mode 0/result -1,
Weapons cursor 0, the first offer panel, and Start returning to the field.
The QA also checks that Wire-Masters and MicroTronics remain explicit service
boundaries while their mode-specific shops are unfinished. Merlin's Lore is
covered separately by the shared result-zero field-store path.

The whole-screen host captures are written to:

- `decompilation/ghidra_conversion/build/conversion/qt/release/qt_runtime/field-scene5-store-service-entry-qa/weapons_world/menu-category/actual.bmp`
- `decompilation/ghidra_conversion/build/conversion/qt/release/qt_runtime/field-scene5-store-service-entry-qa/weapons_world/first-weapon-offer/actual.bmp`
- `decompilation/ghidra_conversion/build/conversion/qt/release/qt_runtime/field-scene5-store-service-entry-qa/weapons_world/field-return/actual.bmp`

Debug and Release produce identical SHA-256 hashes for these three images.
They are host regression artifacts, not independent Genesis captures. Group
3 weapon-accessory actions, group 4 firearm-accessory actions, transaction
outcomes, and shop audio remain separate source-tracing work.
