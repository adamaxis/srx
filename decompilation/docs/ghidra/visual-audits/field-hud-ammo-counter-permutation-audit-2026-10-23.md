# Field HUD Ammunition Counter Permutation Audit

Date: 2026-10-23  
Primary source: US Genesis ROM `input/shadowrun_genesis_us.bin`  
Scope: three-slot field HUD counter values, equipped firearms, grenade
quantities, and the source redraw/consumption paths.

## ROM path and displayed state

The normal field refresh is ROM `0x39F2` -> `0x38FA` -> `0x36FC`. `0x38FA`
visits all three runner records and passes D1=0 to `0x36FC`. The latter picks
the row's HUD nametable from record type `+0x3E`, then draws counter tiles from
`0x6AC46` using the displayed number as a two-tile row index.

The number is derived from the live record, not copied from one field in all
cases:

- Inactive records (`+0x10` bit 0 clear), negative equipped slots (`+0x56`),
  and the `0x7F` unequipped marker draw `00` and leave the stored ammo byte
  untouched.
- Equipped item codes 1..12 update `+0x4E` with the signed byte delta. A
  result at or below zero becomes zero; a positive result is clamped to
  `D4540 + (item_code - 1) * 0x30 + 0x24` before the HUD row is selected.
- Item codes 13..15 use the selected inventory phase byte `+0x6E[+0x56]` as
  the displayed quantity. A result at or below zero clears that phase and its
  item code at `+0x66`, marks the equipped slot `0x7F`, clears `+0x57`, and
  uploads the field assets. Positive quantities select their corresponding
  two-tile row.
- Codes 16 and higher take the zero-counter path. Active spells use a negative
  equipped slot and therefore also take the zero-counter path.

`0x38AC` is the separate action service. It does nothing when `+0x4E` or
`+0x57` is zero. For record types 0..2 it calls `0x36FC` with -3 for firearm
codes 8..10 and -1 for other equipped item codes. Record types above 2 take a
separate one-unit `+0x4E` decrement path. The field refresh's D1=0 and the
action service's negative D1 are distinct transitions.

The 12 ROM capacities at `D4540 + 0x24` are:

| Code | Item | Capacity |
|---:|---|---:|
| 1 | Streetline Special | 6 |
| 2 | Model 101T Light Pistol | 12 |
| 3 | American L36 Light Pistol | 11 |
| 4 | Security 500 Light Pistol | 25 |
| 5 | Warhawk Heavy Pistol | 6 |
| 6 | Max-Power Heavy Pistol | 10 |
| 7 | Predator Heavy Pistol | 15 |
| 8 | AK-97 SMG | 30 |
| 9 | HK227-S SMG | 28 |
| 10 | Mach 22 SMG | 24 |
| 11 | Allegiance Shotgun | 5 |
| 12 | Roomsweeper Shotgun | 8 |

Class and race do not select the capacity formula. They still matter to the
shown starting value because the class/race profile selects a different
record, equipped item, and initial ammo. For example, Gator Shaman's Human
template equips item 2 and starts `+0x4E=32`; its first field refresh clamps
that count to 12. The tenth recruited template is a Human Samurai with two
Frag Grenade inventory cells, each carrying quantity 6 at source record
`0x7351A`.

## Number graphics and permutation coverage

Raw ROM rows `0x6AC46..0x6ACE1` contain 31 full two-word entries for values
0..30. Both words for every row match the named host tile row. Weapon capacity
bytes match both `kFieldHudAmmoTypeCaps` and the typed D4540 item catalog.

The new `shadowrun_ghidra_field_hud_ammo_permutation` CTest reads the raw ROM
and checks:

- All 12 capacity bytes and all 62 counter-tile words against their host
  tables.
- 46,080 firearm update states: 12 weapon codes, all 256 ammo byte values,
  all three party-record rows, and deltas -3, -1, 0, +1, and +3. This checks
  signed wrap, empty-state clearing, capacity clamps, and the selected number
  graphics.
- 36,864 inactive/unequipped states over all ammo bytes, all three party rows,
  both clear-active flag forms, and both negative / `0x7F` equipped markers.
- 71,680 known non-ammunition item states for catalog codes 16..50, eight
  equipped slots, and all `+0x4E` byte values.
- 9,216 `0x38AC` firearm action states, including the three-round service for
  AK-97, HK227-S, and Mach 22.
- 6,624 inventory-phase display states and 744 grenade service states across
  Frag, Scatter, and Concussion Grenades, all eight slots, all three party
  rows, and source-valid quantities through the counter table's value 30.

The Qt HUD QA also writes
`build/conversion/qt/{debug,release}/qt_runtime/field-hud-qa/`
`party-grenade-equipped/actual.bmp`. It starts from source record `0x7351A`,
checks the source slot-3 item and quantity, sets that slot as equipped in typed
state, then requires the Frag Grenade art and the `06` counter asset at the
traced panel coordinates. The Debug and Release
screenshots have the same SHA-256:
`43EA7CB40531AAFCA61B65F18945B91FF6717481429C3BFA9AC6D59FF0FEA00F`.
This is a source-template host-render check; the QA setup does not drive a
physical equip input for this recruited character.

## Visual comparison and limits

Independent longplay frame
`video analysis/long_gameplay_video_early_sections_ffmpeg_30fps_320x224/section_04_field_corridor_npc_dialogue_0238-0390/frame_000900.png`
shows a Streetline counter of `05` in the active HUD row and `00` in the empty
companion rows. It supports the number placement, active-row distinction, and
two-digit presentation. It does not identify the underlying save record or
prove the entire capacity table.

The generated grenade frame shows the correct source item and quantity in the
same HUD region. The supplied video has no identified matching equipped-
grenade frame, so that visual state remains a host check against the source
record and the ROM-selected graphics table. Counts above 30 are outside the
number table and are not claimed as valid inventory quantities here. The
combined reported `32/37/4` weapon-transfer state still needs a matching
normal-play record or independent capture.

## Disposition

- **Runtime behavior changed:** none in this audit slice.
- **Audit coverage added:** raw-ROM capacity/tile comparisons, the six-state
  counter/action matrix, and a full-frame source-template grenade HUD image.
- **Debug and Release:** the new workbench CTest and field-HUD Qt QA pass in
  both configurations. Debug and Release grenade screenshots match by hash.
- **Independent visual evidence:** section 04 frame `000900` shows an active
  five-round Streetline counter and empty companion counters. It does not
  show the same source profile or a grenade counter.
- **Still partial:** recruited-party route integration, source-valid grenade
  equipment input, exact 32/37/4 reconciliation, and broader field-HUD states
  remain open.

See the prior [HUD ammo/capacity audit](hud-ammo-capacity-refresh.md),
[combat-bar profile audit](equipment-combat-bar-profile-permutation-audit-2026-10-23.md),
and ROM entry `0x36FC` in `decompiled-function-status.tsv`.
