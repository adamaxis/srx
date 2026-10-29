# ROM source record extracts

These small TSV files support reproducible extraction of typed gameplay
records. The primary input is `input/shadowrun_genesis_us.bin`, the 2 MiB US
Genesis ROM with SHA-256
`92a614491fc3ea12a6df32459cce08b48574b7b814e244c07eb860dc5c6edb7e`.

`items_full.tsv` records the 50 inventory/equipment rows at ROM `0xD4540` and
the 14 spell rows at `0xD4EA0`. Its pointer and numeric columns are source
evidence; `kind_repo` is the gameplay category used by the item system, since
that category is not stored as a single byte in the item record. The final
`meaning` column is explanatory evidence, not generator input.

From the repository root, regenerate ROM-backed Lua row fragments with:

```powershell
python tools/gen_records.py items
python tools/gen_records.py spells
python tools/gen_records.py accessories
python tools/gen_weapons.py
```

The item/spell/accessory generators read the ROM records and this TSV's item
category labels. The weapon generator reads the item records plus sound tables
`0x15318` and `0x15336`. The generators translate source layout into named
gameplay properties before emitting Lua. Weapon capacity and hit behavior live
in the weapon catalog, armor has named primary/secondary modifiers, item and
spell target approach data is expressed as world-distance intervals, spell
drain inputs are resolved through `0x166AD4`, and totems expose per-spell
bonuses calculated from `0x104F6`/`0xC8AE0`. ROM `0x12A8C` reads live actor
detail values for the extended item-action bands, so the importer does not
guess static "uses" from unrelated `+0x24` bytes. It also omits totem prices:
their `+0x22/+0x24/+0x26` words are modifiers, not purchase costs. Runtime Lua
records use stable item/spell IDs and do not receive record offsets, pointer
addresses, or `b24`-style fields. Edit source inputs and regenerators when
findings change instead of patching generated rows by hand.
