# Armor Selection and Atk/Def Profile Permutation Audit

Date: 2026-10-23  
Primary source: US Genesis ROM `decompilation/input/shadowrun_genesis_us.bin`  
Typed route: command-overview inventory selection and `+0x76` armor state  
Qt QA: `shadowrun_ghidra_conversion_qt_field_profile_combat_bar_permutation_qa`

## ROM-to-screen path

ROM `0x12148` maps the inventory coordinates to `row * 2 + column`. Armor
item codes `0x2C..0x32` map to armor ordinals 1..7. At `0x1219C`, an Equip
action with any armor already active clears runner `+0x76` and the selected
cell's phase byte at `+0x6E + slot`. A later Equip action takes `0x121B0`
and writes the selected ordinal to both locations. With no armor active, the
first action equips the selected row; a repeat clears it.

The selected-item detail path reads D4D50 `+0x25/+0x26`; the equipped command
page summary reads the active `+0x76` row. The tested ROM values are:

| Item code | Armor ordinal | D4D50 `+0x25/+0x26` |
| --- | ---: | ---: |
| `0x2C` | 1 | 2 / 1 |
| `0x2D` | 2 | 1 / 2 |
| `0x2E` | 3 | 5 / 3 |
| `0x2F` | 4 | 4 / 3 |
| `0x30` | 5 | 3 / 2 |
| `0x31` | 6 | 6 / 4 |
| `0x32` | 7 | 8 / 6 |

The Off/Def display uses a separate source path. `0x12CEE` reaches the
`0x55E62` action modifier; that path checks action flag bit 5 or the first
inventory item-code-`0x27` phase. It does not read equipped armor `+0x76`.
The combat hit path at `0x55948` does use the target's `+0x76` to choose the
D4D50 primary or secondary modifier, then applies the action defense modifier.
The Qt audit checks the calculated Atk/Def values and named bar pixels after
each armor action, so the display remains tied to its actual inputs.

One other numeric transition occurs before the menu opens: the field refresh
path `0x39F2 -> 0x38FA -> 0x36FC` clamps an active weapon's `+0x4E` rounds to
the D4540 `+0x24` capacity. Joshua Samurai's source record starts with item 2
and 32 rounds; its first field refresh clamps that to the ROM capacity of 12
at `0xD4570+0x24`. The QA now checks that transition from raw profile bytes and
the primary ROM table before using the refreshed record as its armor-action
baseline. The ammo change is the HUD refresh behavior, not an armor side
effect.

## Profile and action permutations

The QA reads all thirteen 0x100-byte runner profiles from ROM locations
`0x7291A` and `0x72C1A`, and compares each loaded source record byte-for-byte
with its named asset. For every profile it selects all seven armor rows by
controller input: 91 profile/item cases total. It verifies the inventory
cell, item palette and icon, selected D4D50 preview, equipped-cell marker,
`+0x76`, selected `+0x6E` phase, and the equipped command-page summary.

The action route checks A to open the Equip row, A to dispatch event `0x24`,
the source clear-then-equip behavior when armor is already active, repeat
unequip, and the one-step equip/repeat-unequip route for the source profile
that starts without armor. It checks that the other source profile fields,
inventory cells, weapon state, and refreshed ammo remain stable. Each of the
91 cases checks that Atk/Def values and the named bar images remain unchanged
by armor selection or `+0x76`; it also checks Start return, menu re-entry, and
the resulting field frame.

Debug and Release CTests pass. Each configuration writes 539 320x224 armor
images: 91 selected, 168 replacement-clear, 91 equipped command, 91 equipped
field, 91 unequipped field, and 7 no-armor baseline pages. Together with the
existing posture/equipment captures, each QA output has 643 BMPs. All 643
relative filenames and SHA-256 hashes match between Debug and Release; the
91-row case manifest also matches. No production runtime code changed in this
audit; the changes are QA assertions and captures.

Representative host captures are under
`decompilation/ghidra_conversion/build/conversion/qt/debug/qt_runtime/field-profile-combat-bars/armor-profile-permutation/`:
`troll-samurai-armor-44-selected.bmp`,
`joshua-samurai-armor-45-replacement-cleared.bmp`,
`joshua-samurai-armor-45-equipped-command.bmp`, and
`joshua-samurai-armor-45-unequipped-field.bmp`. They verify the current Qt
layout and ROM-derived state; they are not independent Genesis goldens.

## Independent visual check and disposition

Longplay section 06 frame `002560`, at
`video analysis/long_gameplay_video_early_sections_ffmpeg_30fps_320x224/section_06_combat_inventory_delivery_0510-0600/frame_002560.png`,
shows Armor Clothing selected, its one-pixel inventory cursor, and the lower
Wpns/Guns detail panel. That independently supports the selected-armor panel
placement and shared inventory shell. The supplied recording does not show
the exact clear/equip/repeat sequence for these 91 profile/item states, so it
does not verify the armor state transitions or all profile presets visually.

**Disposition: partial.** The source-driven state/action permutations and
internal named-asset placement checks pass in Debug and Release. Keep the
inventory/equipment and Atk/Def hills partial until an independent Genesis
capture supports the armor replacement/equip states. See the [equipment
combat-bar audit](equipment-combat-bar-profile-permutation-audit-2026-10-23.md)
and [inventory selector review](item-selector-and-unequip-review.md).
