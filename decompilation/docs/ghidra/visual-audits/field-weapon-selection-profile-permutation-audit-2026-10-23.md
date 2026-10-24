# Field weapon selection and ammo profile audit

## ROM path

The primary US ROM trace starts at `0x12148`: selector bytes `FFF0D0/FFF0D1`
choose inventory cell `+0x66 + 2*row + column`. `0x12168` dispatches weapon
codes 1..12 to `0x121E0`. The selected inventory bytes `+0x56/+0x57` identify
the equipped slot and item; the old magazine and shared reserve live at `+0x4E`
and `+0x4F`.

When the previous active item is a weapon, `0x121E0` calls `0x125FA`. That
routine reads the old capacity at D4540 row `(item_code - 1) * 0x30 + 0x24`,
computes `ceil(rounds * 100 / capacity)`, clears `+0x4E`, and returns the
remaining percentage. A same-cell selection stores the percentage in `+0x4E`
and writes the `0x7F` unequipped slot sentinel. A different weapon passes the
percentage through `0x12256`, multiplies by the target row's `+0x24` capacity,
divides by 100, and rounds half up when the remainder is at least 50. The
shared Universal Gun Clips reserve `+0x4F` is not changed by weapon selection.

`0x36FC` applies the live count and clamps it against the active weapon's
D4540 capacity before the counter glyph is drawn. An unequipped percentage
stays in `+0x4E` while the HUD counter displays `00`. The inventory redraw at
`0x11624` and item upload at `0x3404` follow the selection change.

The twelve raw capacity bytes at D4540 are:

| Item code | Capacity | Item code | Capacity |
| ---: | ---: | ---: | ---: |
| 1 | 6 | 7 | 15 |
| 2 | 12 | 8 | 30 |
| 3 | 11 | 9 | 28 |
| 4 | 25 | 10 | 24 |
| 5 | 6 | 11 | 5 |
| 6 | 10 | 12 | 8 |

## Race/class and action coverage

The QA verifies all thirteen source records byte-for-byte against ROM
`0x7291A..0x7351A`, preserving their source race, class, attributes, skills,
armor, cyberware, flags, clip reserve, and existing inventory. Their actual
starter weapons are codes `2,3,2,1,11,1,3,5,4,unarmed,7,10,9`; the source
Human Samurai-1 unarmed state uses slot sentinel `0x7F`, item code 0, rounds 0,
and clips 0. The other starter templates begin at `+0x4E=32`, and the field
refresh clamps each to its own capacity. A controlled distinct target item is
inserted into an empty cell for the physical controller route; target codes
cover all twelve weapon rows across the thirteen profiles.

The raw-formula permutation executes 29,952 typed transitions: every old
weapon code, every legal old-round count from zero through that capacity, every
target weapon code, and each of the thirteen race/class records. Its independent
oracle follows the two traced ROM division branches; the manifest groups these
as 1,872 profile/old-weapon/target-weapon pairs.

The physical route uses ordinary inputs to select the target, open the default
Equip action, switch weapons, open the command combat page, return with Start,
unequip the same item, and re-equip it. It checks the exact `+0x4E` count or
percentage after each action, unchanged `+0x4F`, race/class facts, and named HUD
counter pixels. The combat page checks the three weapon-code skill bands from
`0x12C74`: codes 1..7 compare skill bytes `+0x81/+0x82`, 8..10 compare
`+0x81/+0x83`, and 11..12 compare `+0x81/+0x84`. At Full Defense the test
independently calculates the selected skill value (minimum one) and checks the
named Atk/Def bars. The Defense expectation continues to include the source
attribute average, Body, item `0x27` phase or flag priority, and the first
applicable cyberware bit.

## Screens and evidence limits

Debug and Release `shadowrun_ghidra_conversion_qt_field_profile_combat_bar_permutation_qa`
pass. The physical profile output contains `profile-weapon-cases.tsv`,
`profile-weapon-ammo-matrix.tsv`, and 117 full-screen BMPs in each build; all
117 relative filenames and SHA-256 hashes match. The matrix contains 1,872
pair rows representing 29,952 legal-round transitions.

I inspected the Joshua Samurai code-1/6-round page, Bear Shaman code-8/30-round
page, Elf Mage code-11/5-round page, and Human Samurai-1 unarmed-to-code-10
`00` display at 320x224. Their bar and counter regions match the exact named
assets selected by the test. These are internal host checks, not independent
Genesis proof. Independent section 06 frame `002560` shows Armor Clothing in
the shared inventory shell. The 2,700 section-06 and 3,600 section-07 frames
were compared against the inventory states and changing samples were reopened
at full resolution. The visible changes are Armor Clothing, Stim Patch/Medkit
details, First Aid target and result pages, and the return to field; no weapon
selection, equip/unequip, clip reload, or post-switch Atk/Def screen occurs in
the supplied footage. The count sequence is recorded in
`field-item-detail-value-phase-audit-2026-10-23.md`. Keep the weapon hill
partial.

Artifacts:

- `build/conversion/qt/{debug,release}/qt_runtime/field-profile-combat-bars/weapon-profile-permutation/`
- `build/conversion/qt/{debug,release}/qt_runtime/field-profile-combat-bars/armor-profile-permutation/`
- Existing First Aid count evidence is in
  `visual-audits/field-item-detail-value-phase-audit-2026-10-23.md`.

Next audit: capture the final Medkit depletion and no-target branches through
the controller, then continue to another incomplete UI sequence. The inventory
and combat pages still need independent weapon-switch/reload visuals.
