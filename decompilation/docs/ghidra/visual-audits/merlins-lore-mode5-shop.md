# Merlin's Lore mode-5 shop review

**Status:** Partial. Scene-5 event 3 now reaches the shared Lua field-store
presenter after its ROM-backed prompt. The mode-5 catalog and ROM progression
training gate are covered in Debug and Release. The remaining magic item and
focus actions, successful progression purchase, audio, and independent Genesis
screen comparison remain open.

## ROM trace and catalog

Scene-5 event 3 is intercepted by `0x1FA78` and selects the
`merlins_lore` field service. Accepting its prompt calls the field service
loop at `0x56AC0`; the result-zero action path selects the four headings at
`0x1AAAA7`. Mode 5 selects progression records from `0x1AAF80`, magic item
records from `0x1AAF28`, and focus records from `0x1AAEE0`.

| Group | ROM row | Records |
| --- | --- | --- |
| Spell progression | `0x1AAF80` | `07/06/03E8`, `08/06/03E8`, `09/06/03E8`, `06/06/03E8`, `0A/06/03E8`, `0B/06/03E8` |
| Magic items | `0x1AAF28` | `12/0A/0D`, `26/04/1E`, `27/03/14`, `27/04/20` |
| Magic foci | `0x1AAEE0` | `25/03/12`, `1D/02/00`, `21/02/00`, `21/04/20`, `22/02/00`, `22/03/16` |

The Lua catalog keeps progression records separate from ordinary item offers.
The reusable C++ field-store adapter selects the spell group from the Lua
definition, retains mode 5 and result 0, and routes its source-owned
progression records to the existing typed progression-cost path.

## Route and captures

`shadowrun_ghidra_conversion_qt_field_scene5_store_service_entry_qa` follows
Scene-5 event 3, accepts the ROM-backed prompt, checks mode 5/result 0 and the
spell category, selects its first progression row, verifies the unavailable
training message for the default runner, and checks Start returns to the
field. Whole-screen host captures are written under:

- `decompilation/ghidra_conversion/build/conversion/qt/release/qt_runtime/field-scene5-store-service-entry-qa/merlins_lore/menu-category/actual.bmp`
- `decompilation/ghidra_conversion/build/conversion/qt/release/qt_runtime/field-scene5-store-service-entry-qa/merlins_lore/spell-training-gate/actual.bmp`
- `decompilation/ghidra_conversion/build/conversion/qt/release/qt_runtime/field-scene5-store-service-entry-qa/merlins_lore/field-return/actual.bmp`

Debug and Release artifacts are internal regression captures. They do not
establish visual fidelity against the Genesis game.
