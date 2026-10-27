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
is compressed, so it is not an exact pixel golden. Comparing each independent
104x8 strip against every extracted ROM bar row ranks Attack row 5 first
(normalized correlation 0.916, mean absolute channel error 8.52/255) and
Defense row 4 first (0.961, 7.46/255). The Qt crops match those two named
assets exactly. This independently corroborates the formula-derived values
without claiming pixel identity. The capture shows Mental 10%; the QA sets the
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

## Upper bar range check (2026-10-26)

The prior permutations use source presets and valid spell forces, but they do
not drive command-bar inputs to their trained attribute/skill ceilings or the
highest acquired Combat Sense Spell Lock phase. That leaves high-row
presentation unverified.

ROM `0x12CEE` subtracts one from the computed value, clears only a negative
result, multiplies by `0x1A`, and adds it to the selected bar base. It has no
upper clamp. The Attack base `0xCF3D4` and Defense base `0xCF610` are exactly
22 rows (`22 * 0x1A`) apart, so Attack index 22 is the same source row as
Defense index 0. Attack index 23 is likewise the same row as Defense index 1
at `0xCF62A`.

The candidate high values follow from the source bounds:

| Race | Quickness + Intelligence + Willpower ceilings | Base command average | With Spell Lock phase 4 |
|---|---:|---:|---:|
| Human | 6 + 6 + 6 | 9 | 13 |
| Elf | 7 + 6 + 6 | 9 | 13 |
| Dwarf | 5 + 6 + 7 | 9 | 13 |
| Orc | 6 + 5 + 6 | 8 | 12 |
| Troll | 5 + 4 + 5 | 7 | 11 |

The attribute ceilings are the raw race table at `0xC22A6`; skills can be
trained to 12. At Full Offense, `0x55F20` contributes the full command average.
The `0x12C74` merge adds the selected skill and the lesser of that skill and
the posture value. A phase-4 item plus rank-12 action therefore yields Attack
24 for Human, Elf, Dwarf, or Orc, selecting bar index 23. Merlin's Lore has
two item offers with raw selector `0x27` and phases 3 and 4. Purchase code
`0x57A2E..0x57A36` increments that selector to inventory item `0x28` and copies
the offer phase into the record. This connects the offered item to the Combat
Sense Spell Lock row `0x28` and the phase consumed by `0x55ED6`.

The conversion previously clamped every bar to index 22 and Qt loaded only
rows 0..22. That made Attack 24 present row 22 (Defense 0) where the ROM
pointer selects row 23 (Defense 1). The Attack conversion now preserves the
source row index; Qt has a 24th Attack image slot that aliases the existing
named Defense-1 asset. The core test checks the race-ceiling/phase-4/rank-12 formula,
and the Qt profile QA drives the command page and checks the full Attack-24
strip pixel-for-pixel against `defense-bar-1.bmp`. Its host frame is
`qt_runtime/field-profile-combat-bars/upper-range-attack-24-human.bmp`. Both
Debug and Release unit/Qt profile tests pass; the two host frames have the
same SHA-256 hash.

The legal Defense maxima by race are now traced through the acquisition
gates. At Full Defense, the command average contributes its full value; Body
is then added as a low byte, the first matching defense modifier applies, and
the first installed cyberware bit 9..11 adds 1..3. With no subtracting flag or
item `0x27`, phase-4 item `0x28`, race-ceiling attributes, and Dermal Plating
tier 3, the maxima are:

| Race | Max command average | Max Body | Tier-3 bonus | Max Defense | Row index | ROM row address |
|---|---:|---:|---:|---:|---:|---:|
| Human | 13 | 6 | 3 | 22 | 21 | `0xCF832` |
| Elf | 13 | 6 | 3 | 22 | 21 | `0xCF832` |
| Dwarf | 13 | 7 | 3 | 23 | 22 | `0xCF84C` |
| Orc | 12 | 9 | 3 | 24 | 23 | `0xCF866` |
| Troll | 11 | 11 | 3 | 25 | 24 | `0xCF880` |

The last row is a source-derived reachable state for the actual Troll Samurai
template at `0x72D1A`. Its initial Body is 7, its `+0x7E/+0x8E` Essence is
60, and inventory slots 4..7 are empty. `0xD82A` selects the six race caps
from `0xC22A6`; it applies no class restriction to Body, and Body 7 to 11
costs 38 accumulated Karma under the `0xD3EA` next-value cost rule. Merlin's
Lore's raw item records at `0x1AAF28` include `27/04/20`; purchase at
`0x5794E..0x57A36` increments selector `0x27` to inventory item `0x28` and
copies phase 4. The purchase path has no race/class branch. Wire-Masters
category 6 at `0x571B2..0x5739C` advances Dermal Plating through bits 9, 10,
and 11, with each tier replacing the prior bit. Its three `0x1AA8A0` meter
costs are 5 each, so this template retains positive Essence after the tier-3
purchase. The general acquisition checks still require the source funds and
Karma; the QA seeds the acquired maximum state rather than playing those
separate purchase and training screens.

The ROM has no upper Defense clamp at `0x12E7A..0x12E88`. Its dedicated
Defense table contains 22 rows, indices 0..21, from `0xCF610` through
`0xCF84B`. Higher legal indices follow the same 0x1A-byte pointer arithmetic
into adjacent source name words: index 22 starts at `0xCF84C`, index 23 at
`0xCF866`, and index 24 at `0xCF880`. These blocks overlap the later Damage
and armor-summary source data, but the ROM still reads 13 name words from each
computed address. The offline importer now turns those exact row addresses
into named Defense assets; the runtime consumes those images directly.

Reward command `0x133A6` can assign phase 2..6 to a non-weapon item, but its
only caller is `0x1DF34`. The weighted tables selected through `0x16CC8` point
to `0x16D04`, `0x16D18`, `0x16D28`, `0x16D3C`, and `0x16D50`; none contains
reward selector `0x27` for inventory item `0x28`. The item-specific maximum
phase established in these traced acquisition paths is therefore the shop's
phase 4 offer, not the generic reward routine's phase 6 ceiling.

The Attack-24 and Troll Defense-25 cases enter the Qt command menu through its
controller route and verify the displayed strips against the named assets.
The Troll case starts from the exact ROM template, then seeds the trained
attributes, phase-4 item, and tier-3 cyberware flags whose purchase paths are
traced above. The full-frame Debug and Release outputs match: Defense 25 is
`26958E3EA9EAAD462A0006FED638630085F08ED936A85A59FC852D2B9E4A5998`, and
Attack 24 is
`FF63B904CA5C0BD208EA16BECE658465CECBB24AE017B303FC6A3BB5CAC6DD84`.
These are internal renderer checks; no independent Genesis frame shows these
high-value states. The source calculation has no upper clamp, while the Qt
presenter has named rows only through index 24 and skips drawing an index past
that array.

The higher-mode Attack bound follows a different path, and its legal maximum
is now traced. ROM `0x12D7C..0x12DAC` calls `0x12FA4`, starts from `0x55F02`
(Sorcery `+0x80` plus the first Power Focus phase in slots 6..0), selects the
posture reduction with `0x55F20`, caps it at Magic resource `+0x7F`, adds
current Force `+0x9D` as a byte, and calls `0x104F6` for spell-focus and totem
modifiers. `0x104F6` adds the matching spell-focus phase from slots 6..0; if
slot 7 contains Rat, Gator, or Bear, it indexes the category at `0xC8AE0` and
adds the signed word at `0xD4540`. The only post-modifier floor is one; the
path has no maximum clamp.

The legal inputs bound that unclamped calculation. The source spell-force
rows selected by `0x100A8` from the five tables at `0x166AD8` top out at
Force 4. The source spell-focus offer table at `0x1AAEB0`, read by the field
purchase path `0x5794E..0x57A36`, gives Mana Storm's matching item code `0x22`
phase 4 at `0x1AAEE9`; the other matching spell-focus offers top out at phase
3. The separate phase-4 selector `0x27` at `0x1AAF1C` writes item `0x28`
(Combat Sense Spell Lock), which is not the `0x19 + spell code` item read by
`0x104F6`. The traced reward tables do not grant spell-focus item codes
`0x19..0x26`. Power Focus purchase `0x22C46` writes item `0x18` at phase 2.
The `0xD82A` skill-upgrade route permits Shaman Sorcery to rank 12,
which is covered by the profile permutation audit. The spell-capable source
profiles have Magic cap `+0x7F = 6`; the live-record writer scan finds template
initialization and the cyberware update at `0x573D4`, which lowers the cap but
cannot raise it. At Full Offense, `0x55F20` uses the full Sorcery plus Power
Focus value before that cap, so the largest contribution is 6.

Crossing those source limits with the slot-7 totem selector and modifier rows
at `0xC8AE0`/`0xD4540` leaves Orc Shaman Mana Storm as the highest legal
combination: Sorcery 12 plus Power Focus phase 2 is capped to 6, Force 4,
Mana Storm Spell Focus phase 4, and Bear Totem's selector-1 modifier is zero.
That produces Attack 14 and row index 13. The positive Rat/Gator `+2`
categories do not coincide with a Force-4, phase-4 spell-focus combination;
the strongest checked Rat case is Stink at 13 (Force 3, phase 2, plus 2),
and the strongest no-totem Elf Mage cases are also 13 (Force 4, phase 3).
The unit test seeds the exact source profile and item phases, verifies the
independent source-formula oracle, and the Qt QA uses controller input to
select Mana Storm, accept Force 4, enter posture, raise the source posture to
Full Offense, confirm, return to the command page, and select the combat tab.
The resulting live fields are `+0x56=-1`, `+0x57=10`, `+0x9D=4`, and
`+0x4D=4`; the QA checks row 13 against its named asset and a clean return.
It also verifies the command shell, HUD chrome, and Mana Storm title pixels in
the saved frame.
The internal full-frame capture
`qt_runtime/field-profile-combat-bars/higher-mode-special-attack-14-orc-shaman-bear-totem.bmp`
hashes to `C186FA726BFCC9ED15EB1635251A1B727B4A5B558DF89F4923F4C34E21D68D89`
in both Debug and Release. This bounds the calculation while leaving
independent Genesis footage for the high-value spell state open.

## Force-dependent Power label and Mana Storm sweep (2026-10-26)

In higher source mode, `0x1303C..0x13042` reads the active runner's Force at
`+0x9D` when `+0x56` is negative. `0x13094..0x130B4` subtracts one, clamps
the selected row to 0..12, and draws the corresponding 12-byte label from
`0xD1F5E`. Thus Force 1 selects `Meager`, Force 2 `Minor`, Force 3 `Very Low`,
and Force 4 `Low`. Force input at `0x100A8` is bounded below by one and above
by the selected spell's maximum.

The Qt QA now reaches all four Force states through the controller on the
source Orc Shaman/Bear Totem profile. It selects D4EA0 spell code 9 (Mana
Storm), whose table-2 Force limit is 4 in `0x166AD8`, enters the Force panel,
uses Left to reach each lower value, enters Full Offense, completes the spell,
and returns to the combat page. It checks the rendered Force digit at
`(216,104)`, the Power text at `(168,88)`, the Mana Storm title, exact bar
assets, the published live Force, and clean field return. The same frames
check Mana Storm's D4EA0 `+0x25` Damage row (`damage-bar-2.bmp`) and whether
the `0x10660` residual should overlay `None` or leave the positive bar visible.

With Sorcery 12, Power Focus phase 2, Magic cap 6, Full Offense posture,
Mana Storm focus phase 4, and Bear's selector-1 modifier 0, the special
`0x12CEE` formula yields Attack `10 + Force`: 11, 12, 13, and 14, selecting
rows 10..13. Debug and Release pass the physical route. Four Force-panel and
four completed-command frames have matching SHA-256 hashes between
configurations. They are internal captures, not independent Genesis
goldens. Independent section-06 frames `000015`/`000020` show Gator Mana Zap
Force 3 / Neutral in the `Set Force & Posture` panel; its panel, digit, and
meter placement agree with the internal Force-3 panel. Frame `002550` shows
the parent Gator command page's Power `Very Low` and Damage `None`. These
compressed frames support only the Gator/Mana Zap states and nearby layout;
they do not show Mana Storm or Forces 1, 2, or 4. The hill stays partial.

## Disposition

- **Code changed:** corrected the item `0x2B` D4540 word; connected the active
  runner's Sorcery, Magic cap, force, spell row, item catalog, and C8AE0
  selector to the Qt combat-bar calculation; rendered the D4EA0 spell title
  and source Damage/Power detail instead of the unrelated D4540 weapon text;
  preserved high Attack row indices and mapped Attack indices 23 and 24 to
  the existing Defense-1 and Defense-2 assets. The Defense calculation now
  preserves the ROM's unbounded row index, and the runtime loads named rows
  through legal index 24; the higher-mode spell calculation's source maximum
  is Attack 14/index 13.
- **Audit coverage changed:** added source-formula expectations and a real
  weapon-Equip input for all 13 source profiles; the 2026-10-26 upper-range
  check fixes the Attack-24 row mismatch and traces the legal Defense range
  through Troll Defense 25. Core expectations cover all five race ceilings;
  the Qt route uses the Troll Samurai template and checks the full Defense-25
  strip and clean return. The new special-action case uses the Orc Shaman
  source profile with Bear Totem, source Sorcery/Force caps, and ROM-verified
  focus items/phases seeded as acquired inventory. Physical input selects Mana
  Storm, Force 4, and Full Offense, then the live action fields yield Attack
  14/index 13 in Qt and the independent formula oracle. The field equipment
  purchase resolver was separately corrected for ROM Dermal Plating bits
  9..11; that change is documented in
  the [Wire-Masters audit](wire-masters-mode7-cyberware.md).
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
- **Upper-range verification:** `shadowrun_ghidra_equipment_combat_bar` and
  `shadowrun_ghidra_conversion_qt_field_profile_combat_bar_permutation_qa`
  passed in Debug and Release. QA reported Attack 24/index 23 and Defense
  25/index 24, matched each strip to its named asset, and verified a clean
  field return. It also checks the source maximum Orc Shaman Mana Storm Attack
  14/index 13 and its clean return. All three upper-range full-frame captures
  match between configurations by SHA-256.
- **Independent visual check:** section-06 frames `000015`/`000020` show the
  Gator Mana Zap Force-3 / Neutral `Set Force & Posture` panel; frame `002550`
  shows its parent command page's Power `Very Low`, Damage `None`, and nearby
  bar rows. Other race/class profiles, Mana Storm, and Forces 1, 2, and 4 have
  no independent matching frames.
- **Still partial:** only source weapon 1 is equipped through the UI in this
  profile sweep; every weapon, armor, gear, spell, posture combination, and
  equipment change route is not yet physically permuted. Independent Genesis
  footage remains limited to one Gator Shaman spell state; the compressed bar
  comparison is approximate. Callers, overhead combat, AI posture, and other
  source-mode branches need separate audits. Defense indices 23..24 now use
  the source name words at `0xCF866`/`0xCF880`, and the Troll high-row capture
  checks index 24; Attack index 24 aliases Defense row 2, while the legal
  higher-mode spell maximum is row 13. Independent Genesis footage for the
  Attack 14 spell state and other high-value rows is still absent. Indices
  above the 25 loaded rows are not drawn by the Qt presenter.
  The all-profile frames confirm Qt presentation against the recovered source
  values but cannot substitute for independent Genesis captures. Keep ROM
  entries `0x12CEE` and `0x12EAE` partial.

See also [posture focus and bars](posture-menu-focus-and-bars.md), the
[visual audit index](../visual-analysis/qt-visual-audit.tsv), and the
`0x12CEE` / `0x12EAE` rows in `decompiled-function-status.tsv`.
