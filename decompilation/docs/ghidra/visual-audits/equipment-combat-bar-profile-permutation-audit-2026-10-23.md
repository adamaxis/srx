# Equipment Combat-Bar Profile and Action Audit

Date: 2026-10-23  
ROM: US Genesis Shadowrun  
Scope: command-overview Atk/Def bars across runner presets, posture, action
selection, spell force, equipment phases, flags, and cyberware

## ROM-to-screen path

ROM `0x12CEE` updates the command-overview Attack and Defense strips. Its
ordinary path calls `0x55ED6` for the command average, `0x55F20` for posture,
`0x12C74` for action-specific skill selection, and `0x55E62` for the Defense
action modifier. The live runner fields are exposed to the calculation through
`EquipmentCombatInput`; `FieldSceneRuntime::refresh_command_overview_combat_bars`
projects the active typed runner record and the presenter draws the selected
named `attack-bar-N.bmp` and `defense-bar-N.bmp` assets at (144,104) and
(144,112).

The main inputs are the active record's posture `+0x4D`, selected/equipped
state and code `+0x56/+0x57`, inventory item codes `+0x66..+0x6D`, item phases
`+0x6E..+0x75`, Body `+0x77`, command attributes `+0x78/+0x7B/+0x7C`,
Sorcery `+0x80`, action skills `+0x81..+0x85` and `+0x87..+0x8C`, action
flags `+0x11`, Magic cap `+0x7F`, cyberware flags `+0x90`, race `+0x9A`,
class `+0x9B`, and current Magic force `+0x9D`.

`0x55ED6` adds `+0x78`, `+0x7B`, and `+0x7C` with byte wrapping, shifts once,
then adds the phase of the first item-code-`0x28` possession. `0x55F20`
applies 25 percent per posture step using integer division. For ordinary
actions, `0x12C74` selects a source skill from the action code and merges it
with the posture-adjusted value; direct-skill branches keep their own source
rules. This means race and class do not directly select a combat-bar formula:
their different presets, skills, inventory, and current runner state feed it.

When `+0x56` is negative in the higher source mode, `0x12CEE` takes the
special action path. `+0x57 - 1` selects the D4EA0 spell row. The attack value
starts from Sorcery `+0x80` plus the first matching Power Focus `0x18` phase
in slots 6 through 0, takes the posture-reduced component, applies the Magic
cap `+0x7F`, then adds the selected force `+0x9D`. The source then adds the
first matching spell focus phase for item `0x19 + spell code` in slots 6
through 0 and the slot-7 Rat/Gator/Bear modifier. `0xC8AE0` selects the
modifier word at D4540 `+0x22`, `+0x24`, or `+0x26`; a signed result at or
below zero is clamped to one. `0x12FA4` also calls `0x10660` to calculate the
selected spell's effect result.

For Defense, a zero command average returns after Attack and does not write a
Defense strip. Otherwise the remaining posture share is added to Body using
the source low-byte operation. `+0x11` bit 5 subtracts two and takes priority
over the first item-code-`0x27` phase reduction. The cyberware scan checks
bits 9, 10, and 11 in order and stops at the first set bit, adding 1, 2, or 3;
no set bit adds nothing. A zero value selects bar row 0; other values select
value minus one.

The negative `+0x56` display path also changes the title/detail source.
`0x111E8` indexes D4EA0 spell text instead of D4540 item text, and `0x12EAE`
uses the D4EA0 `+0x25` row and `0x13000` force label. The Qt presenter now
renders the spell name, source Damage row, residual `None` when the typed
action result is nonpositive, and Power label from the current force. It uses
the runner's Sorcery, cap, force, item catalog, and selector table when it
calculates the Atk/Def strips.

## Profile and input permutations

The combat-bar test reads the three Joshua archetypes and ten recruited
templates from the extracted source records. It checks the stored race/class
identities and preserves each profile's attributes, skills, inventory, item
phases, flags, and cyberware rather than substituting one generic runner.
Every one is checked at all five reachable posture values (65 profile/posture
cases):

| Source profile | Race / class | Command average | Neutral Atk / Def |
|---|---|---:|---:|
| Joshua Samurai | Human / Samurai | 4 | 6 / 6 |
| Joshua Decker | Human / Decker | 6 | 4 / 5 |
| Joshua Gator Shaman | Human / Shaman | 5 | 4 / 5 |
| Rat Shaman | Dwarf / Shaman | 4 | 4 / 6 |
| Troll Samurai | Troll / Samurai | 3 | 5 / 9 |
| Human Mage | Human / Mage | 5 | 4 / 5 |
| Dwarf Decker | Dwarf / Decker | 5 | 6 / 7 |
| Bear Shaman | Orc / Shaman | 5 | 5 / 9 |
| Elf Decker | Elf / Decker | 7 | 7 / 7 |
| Human Samurai | Human / Samurai | 7 | 9 / 9 |
| Elf Mage | Elf / Mage | 7 | 9 / 7 |
| Elf Decker | Elf / Decker | 9 | 8 / 6 |
| Human Samurai | Human / Samurai | 8 | 10 / 12 |

The five actual caster profiles are then checked against each spell in their
ROM-selected `+0xA0` table, every legal force from 1 to that entry's limit,
and all five posture values. The tables contain 21 entries and 52 legal force
states in total, yielding 260 spell/force/posture cases. The limits differ by
class/profile; they are taken from the typed source rows for ROM `0x166AD8`.
The separate [archetype profile audit](archetype-profile-permutation-audit-2026-10-23.md)
and [Magic spell audit](magic-spell-action-permutation-audit-2026-10-23.md)
record the full preset skills, caster tables, and race attribute ceilings at
`0xC22A6`.

The focused flag/equipment checks also cover all eight combinations of
cyberware bits 9 through 11, flag-bit-5 priority, first item-code-`0x27`
selection, low-byte Body wrap, zero-average early return, and distinct lower
mode action gates. These targeted cases do not enumerate every possible
equipment change or every byte value accepted by all action callers.

The new independent `0x12C74` merge sweep checks all 256 action-code bytes,
every four-skill tuple with legal ranks 0 through 12 plus rank 13, the first
training value rejected by the source, and current values 0 through 13 plus
255. Since each selected skill is at most 13, any current value above 13 has
the same merge result as current 13. This gives 147,517,440 checked input
states, including the source's wrapped `CMP.B/BPL` bands, every legal skill
rank, and the first-invalid-rank boundary. It validates the arithmetic
domain; class training gates still determine which synthetic four-skill
tuples a runner can reach.

## Visual cross-check

Independent section-06 frame `002550` shows Joshua as a Human Shaman with
Mana Zap selected, `Damage None`, `Power Very Low`, the posture panel, and
Atk/Def strips. Frames `000015..000020` in the same supplied section show
Mana Zap at force three with Neutral posture. This pins the Gator Shaman's
source context: race 0/class 1, Sorcery 3, Magic cap 6, posture 2, force 3,
and slot-7 Gator totem item `0x2A`. The first matching Power Focus and Mana
Zap spell-focus items are absent. `0xC8AE0[7]` selects word `+0x24`, whose
D4540 item-`0x2A` value is 2. The resulting Attack is 6 (row 5). The command
average is 5; posture leaves 3 Defense before Body 2, with no defense flag,
item-`0x27`, or cyberware bonus, so Defense is 5 (row 4).

The Debug/Release Qt QA builds this typed Gator action state and checks the
title, `Damage None`, force label, exact 104x8 bar assets, and full-frame
capture at `qt_runtime/field-spell-combat-bar-qa/actual.bmp`. The Debug and
Release files have identical SHA-256 hashes. The independent 30 fps capture
is compressed, so the bar comparison is nearest-row corroboration (Atk 5,
Def 4), not an exact pixel golden. It shows Mental 10%; the QA sets the
record's `+0x9C` behavior meter to 1 to match that visible value. The preset's
default meter remains independently tested in the profile sweep.

## All-profile command-screen captures

The new `shadowrun_ghidra_conversion_qt_field_profile_combat_bar_permutation_qa`
loads the three Joshua choices and all ten named recruited records from the
source-derived record assets and verifies each record's race/class identity.
For each record, it opens the command overview with Start, navigates to the
action page with Left, and uses Up/Right/Left/Down through the posture focus.
Only the test fixture's starting posture is normalized to Full Defense; all
other profile inputs remain from that complete record. The source expectation
matrix above is checked at every posture, and the rendered 104x8 strips must
exactly match the corresponding named Atk/Def bitmap at `(144,104)` and
`(144,112)`. Right and Left steps request source sound `0x25`; both endpoints
clamp, Down restores the exact unfocused command page, and Start restores the
exact field frame before the next profile is loaded.

The run writes 65 complete 320x224 host frames per configuration, one for
each profile/posture pair. Debug and Release each pass; all 65 BMP hashes match
between configurations. I inspected Samurai, Troll, Mage, Shaman, and multiple
posture endpoints. These are renderer/regression captures, not independent
Genesis goldens. The supplied independent section-06 frame `002550` still
corroborates only the Gator Shaman Mana Zap state and nearby bar rows. It does
not show the other twelve profile presentations or all five postures for
every profile.

The same QA now reopens the command menu for each record and physically
equips source weapon 1 with Left, A, A, then Right, Right to the combat page.
The test retains each source profile's race, class, attributes, skills, flags,
and other record fields while controlling the selected inventory cell,
posture, and equipped-item state. It independently recomputes the displayed
command average from byte-wrapped attributes and the first item-`0x28` phase;
the Full Defense attack from the item's `+0x81/+0x82` skill pair; and Defense
from Body, the flag-bit-5/item-`0x27` priority, and the first cyberware bonus
bit. Each resulting value and bar index is checked against the live Qt state,
and each strip is compared pixel-for-pixel with its named 104x8 asset. This
connects the actual Equip input to the record mutation, bar refresh, and
displayed pixels for all thirteen profiles.

The expanded run writes 13 post-equip command frames plus pre-action and
post-return field frames for every profile. After the first Start return, the
equipped state is retained; opening and closing the command menu a second time
must reproduce that profile's post-equipment field frame exactly. Debug and
Release pass, and all 104 generated BMPs have matching SHA-256 hashes. The
inspected Troll Samurai field pair shows the expected field-HUD change from
the newly equipped weapon. These remain host captures; no independent Genesis
record of all thirteen profile/equip combinations has been identified.

## Data correction found by the permutation sweep

The Bear Shaman's slot-7 Bear totem is item `0x2B`. The raw D4540 bytes at its
`+0x26/+0x27` action-modifier word are `00 02`, so selector 2 contributes
`0x0002`. The typed item row had incorrectly stored detail selector 2 and
tail 2, producing `0x0202`. Table 2's Heal Wounds action consequently reached
the old bar cap with an attack value of 517 in the sweep. The item catalog now
stores the source bytes correctly, and the test compares all three totem
modifier words directly against constants read from their raw ROM rows before
checking spell permutations.

## Disposition

- **Code changed:** corrected the item `0x2B` D4540 word; connected the active
  runner's Sorcery, Magic cap, force, spell row, item catalog, and C8AE0
  selector to the Qt combat-bar calculation; rendered the D4EA0 spell title
  and source Damage/Power detail instead of the unrelated D4540 weapon text.
- **Audit coverage changed:** added source-formula expectations and a real
  weapon-Equip input for all 13 source profiles; no runtime game logic changed
  in this continuation.
- **ROM behavior checked:** 13 full source profiles across five postures;
  all five caster tables, all 21 listed spells, 52 force states, and all five
  postures; cyberware priority, defense flags/items, byte wrap, zero average,
  spell phase, totem selection, source labels, and force display. The separate
  action-skill matrix checks 147,517,440 combinations over every action byte,
  legal skill-rank tuple, the first-invalid-rank boundary, and merge values.
- **Debug verification:** `shadowrun_ghidra_equipment_combat_bar`,
  `shadowrun_ghidra_equipment_skill_merge_permutation`,
  `shadowrun_ghidra_conversion_qt_field_posture_qa`,
  `shadowrun_ghidra_conversion_qt_field_spell_combat_bar_qa`, and
  `shadowrun_ghidra_conversion_qt_field_magic_spell_qa` passed. The new
  all-profile command-screen CTest also passed, including 13 Equip actions,
  13 post-equip bar screens, and 13 stable field return cycles.
- **Release verification:** the same five tests passed. The Gator combat-bar
  full-frame capture matches Debug by hash. The all-profile command-screen
  CTest passed and all 104 profile/posture/equipment/return capture hashes
  match Debug. The
  exhaustive action-skill permutation test also passes in Release.
- **Independent visual check:** section-06 frame `002550` supports the Gator
  spell title, detail labels, posture, and bar rows. Other race/class profiles
  and spell force levels have no independent matching frames.
- **Still partial:** only source weapon 1 is equipped through the UI in this
  profile sweep; every weapon, armor, gear, spell, posture combination, and
  equipment change route is not yet physically permuted. Independent Genesis
  footage remains limited to one Gator Shaman spell state; the compressed bar
  comparison is approximate. Callers, overhead combat, AI posture, and other
  source-mode branches need separate audits. The all-profile frames confirm
  Qt presentation against the recovered source values but cannot substitute
  for independent Genesis captures. Keep ROM entries `0x12CEE` and `0x12EAE`
  partial.

See also [posture focus and bars](posture-menu-focus-and-bars.md), the
[visual audit index](../visual-analysis/qt-visual-audit.tsv), and the
`0x12CEE` / `0x12EAE` rows in `decompiled-function-status.tsv`.
