# ROM source record extracts

These small TSV files support reproducible extraction of typed gameplay
records. The primary input is `input/shadowrun_genesis_us.bin`, the 2 MiB US
Genesis ROM with SHA-256
`92a614491fc3ea12a6df32459cce08b48574b7b814e244c07eb860dc5c6edb7e`.

`items_full.tsv` records the 50 inventory/equipment rows at ROM `0xD4540` and
the 14 spell rows at `0xD4EA0`. The pointer and numeric columns are ROM values;
`kind_repo` is the gameplay category used by the current item system, since
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
`0x15318` and `0x15336`. Generated fragments are checked against the structured
Lua modules under `gameplay/scripts/data/`; edit the source extraction inputs
and regenerators when source findings change instead of patching generated
rows by hand.
