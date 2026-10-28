# Attributes/Skills Profile and Upgrade-Boundary Audit

Date: 2026-10-23  
ROM: US Genesis Shadowrun  
Scope: Attributes/Skills grid, profile-dependent values, class gates, Karma
preview and purchase, cursor/input permutations, and recruited-template bounds

## ROM-to-screen mapping

ROM `0xD82A` redraws a two-column, ten-row grid. The upper four rows display
eight record bytes from `+0x77..+0x7E`; the lower six rows display twelve skills
from `+0x80..+0x85` and `+0x87..+0x8C`. The label order comes from the 20-entry
`0xC1A2A` table and its descriptions from `0xC1B70`. The currently selected
runner's Karma is the byte at `+0x8D`.

The first six displayed attributes are Body, Quickness, Strength, Charisma,
Intelligence, and Willpower. Essence and Magic occupy `+0x7D/+0x7E` and are
visible in the grid, but `0xD82A` does not offer their upgrade preview. The
separate `+0x7F` Magic resource/cap participates in magic behavior and is not a
ninth grid attribute. `+0x86` is skipped between the two six-skill regions.

`0xD01C` makes the command-menu page view-only (`FFF116 < 0`); its A/B action
returns without mutating Karma or a stat. Hotel Options enters the spending
variant through `0xD024`, which selects the live record and enables the
`0xD82A` preview. At a valid attribute or skill selection, the detail area
shows the next value and Karma cost. `0xD3EA` checks funds, waits for the
confirmation choice, subtracts the cost from `+0x8D`, and writes the selected
next value only after acceptance.

## Source profiles

The table below records the eight values drawn in the upper grid for all
selectable Joshua presets and all ten named recruited runner records. Values
are read from the raw ROM records; the three Joshua records begin at `0x7291A`,
and the ten recruits begin at `0x72C1A`.

| Profile / ROM | Race / class | Body, Quickness, Strength, Charisma, Intelligence, Willpower, Essence, Magic | Magic resource `+0x7F` / spell table `+0xA0` |
|---|---|---|---|
| Joshua Samurai `0x7291A` | Human / Samurai | 4, 3, 4, 1, 2, 4, 3, 5 | 0 / 0 |
| Joshua Decker `0x72A1A` | Human / Decker | 2, 5, 2, 2, 4, 3, 4, 5 | 0 / 0 |
| Joshua Gator Shaman `0x72B1A` | Human / Shaman | 2, 4, 1, 2, 3, 4, 3, 6 | 6 / 4 |
| Rat Shaman `0x72C1A` | Dwarf / Shaman | 4, 2, 3, 1, 3, 4, 2, 6 | 6 / 0 |
| Troll Samurai `0x72D1A` | Troll / Samurai | 7, 2, 5, 1, 2, 3, 2, 6 | 0 / 0 |
| Human Mage `0x72E1A` | Human / Mage | 2, 3, 2, 2, 3, 4, 3, 6 | 6 / 1 |
| Dwarf Decker `0x72F1A` | Dwarf / Decker | 4, 3, 3, 2, 4, 3, 3, 5 | 0 / 0 |
| Bear Shaman `0x7301A` | Orc / Shaman | 6, 3, 4, 1, 3, 5, 3, 6 | 6 / 2 |
| Elf Decker `0x7311A` | Elf / Decker | 3, 5, 3, 3, 6, 4, 5, 3 | 0 / 0 |
| Human Samurai `0x7321A` | Human / Samurai | 5, 5, 6, 3, 5, 4, 5, 2 | 0 / 0 |
| Elf Mage `0x7331A` | Elf / Mage | 3, 5, 3, 5, 5, 5, 5, 6 | 6 / 3 |
| Elf Decker `0x7341A` | Elf / Decker | 1, 7, 3, 5, 6, 6, 6, 2 | 0 / 0 |
| Human Samurai `0x7351A` | Human / Samurai | 5, 6, 5, 3, 6, 5, 6, 0 | 0 / 0 |

The twelve skills displayed in the same profile order are Sorcery, Firearms,
Pistols, SMGs, Shotguns, Melee Combat, Throwing, Computer, Biotech,
Electronics, Reputation, and Negotiation:

| Profile / ROM | Skill values in source label order |
|---|---|
| Joshua Samurai `0x7291A` | 0, 4, 0, 0, 0, 3, 2, 0, 1, 0, 1, 1 |
| Joshua Decker `0x72A1A` | 0, 2, 0, 0, 0, 1, 0, 3, 2, 1, 1, 2 |
| Joshua Gator Shaman `0x72B1A` | 3, 2, 0, 0, 0, 1, 0, 0, 0, 0, 1, 3 |
| Rat Shaman `0x72C1A` | 3, 2, 0, 0, 0, 2, 0, 0, 0, 0, 0, 1 |
| Troll Samurai `0x72D1A` | 0, 2, 0, 0, 4, 4, 1, 0, 1, 0, 0, 0 |
| Human Mage `0x72E1A` | 3, 1, 2, 0, 0, 1, 0, 0, 0, 0, 2, 2 |
| Dwarf Decker `0x72F1A` | 0, 2, 4, 0, 0, 1, 0, 2, 0, 3, 0, 0 |
| Bear Shaman `0x7301A` | 4, 3, 0, 0, 0, 4, 0, 0, 0, 0, 2, 0 |
| Elf Decker `0x7311A` | 0, 1, 4, 0, 0, 1, 0, 5, 2, 3, 1, 0 |
| Human Samurai `0x7321A` | 0, 4, 0, 0, 0, 6, 2, 0, 3, 0, 3, 4 |
| Elf Mage `0x7331A` | 5, 1, 6, 0, 0, 3, 0, 0, 0, 0, 4, 4 |
| Elf Decker `0x7341A` | 0, 1, 0, 4, 0, 0, 0, 6, 3, 5, 3, 0 |
| Human Samurai `0x7351A` | 0, 6, 0, 0, 0, 5, 3, 0, 5, 0, 5, 0 |

These records show the preset differences directly: Samurai rows emphasize
combat skills, Deckers concentrate on Computer/Biotech/Electronics, and
Mage/Shaman rows carry Sorcery plus their distinct spell-table/resource
values. The display reads each profile's fields; it does not infer skills from
the class label.

## Upgrade bounds and cost rules

The raw table at `0xC22A6` supplies six raisable-attribute ceilings by race.
Each race's current starting values above remain below or at these limits;
the upper grid's Essence and Magic fields are outside this six-value table.

| Race index | Race | Body, Quickness, Strength, Charisma, Intelligence, Willpower maximum |
|---:|---|---|
| 0 | Human | 6, 6, 6, 6, 6, 6 |
| 1 | Elf | 6, 7, 6, 8, 6, 6 |
| 2 | Dwarf | 7, 5, 8, 6, 6, 7 |
| 3 | Orc | 9, 6, 8, 5, 5, 6 |
| 4 | Troll | 11, 5, 10, 4, 4, 5 |

This is an increment-only screen; the source has no lower-bound clamp. A
current value of zero can preview an increase to one, subject to Karma. For
attributes, the next value is `current + 1`, its cost is that next value, and
the upgrade is rejected when the next value exceeds the selected race's table
entry. Skills can rise from 0 through 12; `current + 1 >= 13` rejects the
preview. Most skills cost twice the next rank. The left-column Pistol, SMG,
and Shotgun rows use `ceil(1.5 * next rank)`; when one of those current skills
is zero, the source first uses Firearms `+0x81` as the base rank before adding
one. The Sorcery row is restricted when class `+0x9B >= 2` (Samurai and
Decker); Mage and Shaman class values remain eligible.

## Input and display permutation coverage

The native test reads the 13 complete profile records through the named asset
loader and compares every loaded byte against its raw ROM record. It checks all
20 visible cells per record (260 profile/cell combinations), including label
index, record offset, current value, class-restricted state, selected detail,
next value, Karma cost, and callback order. The selected-screen input sweep
visits every cursor location and every 8-bit controller state for each profile
in view-only mode, upgrade-preview mode, and the post-message error state:
199,680 cases. It pins directional/A-B/C/Start priority, row/column clamps,
Sorcery restriction, and the two non-upgradeable attribute rows.

Boundary cases read the five race ceilings from the raw ROM and exercise every
value from zero through one above each maximum (252 cases). Skill checks cover
all four class values, all twelve skill fields, ranks zero through thirteen,
and every Firearms base rank used by the zero-value reduced-cost branch (816
cases). For each available attribute increase, the test checks insufficient
Karma, the unaccepted prompt, and the accepted update to both the stat and
`+0x8D`.

The source table at `0x72C1A` contains ten named recruited records. Record
`0x7361A` has a zero kind word at `+0x0E`; it terminates that list. The next
record-sized byte span is not a runner profile. The offline extractor used to
copy twelve spans, which pulled the terminator and following ROM data into two
of the thirteen host save-template slots. Extraction now stops after ten,
checks the zero-kind terminator, and leaves the final two save slots empty.

## Qt profile-screen permutations

The new `shadowrun_ghidra_conversion_qt_field_profile_attributes_skills_permutation_qa`
loads each of the 13 complete ROM-derived records into the active field runner
and reaches the command-menu view through Start, Left to row 3, then A. The
full 320x224 page header is checked against the profile's race and class. The
test walks every one of the 20 cells with ordinary Up/Down/Right input. At
each cursor position it compares all other visible labels and numeric glyphs
pixel-for-pixel with the raw source record, checks the selected label/value
palette, and validates the grid's record offset and class-restricted state.
This covers all 260 displayed profile cells on the actual Qt screen route.
The command view stays read-only: A returns to command page 6 without changing
attributes, skills, or Karma, then Start redraws the exact field frame. One
complete screen per profile is saved; the Debug and Release sets each contain
13 BMPs with identical SHA-256 hashes.

## Screenshot comparison and disposition

The existing command-page `actual.bmp` shows the Joshua Samurai record, its
eight displayed upper-grid values, twelve skills, and current Karma; the
per-cell Qt dump covers all 20 cursor selections for that source profile. The
new profile-screen set shows the different race/class headers and numeric
presets for all thirteen source records. The Hotel `karma-spend/actual.bmp`
shows Body increasing from one to two at cost two with Karma 50; that matches
the `0xD82A` preview and `0xD3EA` cost/write rules. Those captures are host
screens.

Independent Section-11 frame
`video analysis/long_gameplay_video_rest_ffmpeg_30fps_320x224/section_11_source_1620-1920/frame_002700.png`
shows an Attributes/Skills spending page headed “Joshua / Human Shaman.” It
displays Body 3, Quickness 4, Strength 1, Charisma 2, Intelligence 3,
Willpower 6, Essence 6, and Magic 6; Sorcery 3, Firearms 2, Pistols 0, SMGs 0,
Shotguns 0, Melee Combat 1, Throwing 0, Computer 0, Biotech 0, Electronics 0,
Reputation 1, and Negotiation 3; Karma is 5. Throwing is selected, and the
preview reads “Upgrade to 1 Cost 2.” This is source-consistent: `0xD82A` reads
the selected live record and displays its purchase preview; `0xD3EA` applies
the accepted purchase. Throwing is a generic skill at rank 0, so the next
rank costs twice the next rank (2). Its selected description also matches
the ROM label/description tables.

The Joshua Gator Shaman starter record at `0x72B1A` is Human/Shaman and has
the same twelve visible skill ranks, but its raw attributes are
`2, 4, 1, 2, 3, 4, 3, 6`, and its Karma byte `+0x8D` is 0. The frame therefore
shows a live screen state that differs from the starter values in Body,
Willpower, Essence, and Karma. The screenshot has no record/RAM dump, so it
does not establish when or how those differences arose. This is one
independent visual example, not proof of every profile or upgrade permutation.

- **Code changed:** recruited-record extraction now includes only the ten ROM
  templates before the zero-kind sentinel. The unused two save positions stay
  zeroed.
- **Source behavior covered:** thirteen profile records, 20 grid cells per
  profile, race-specific ceilings, class-specific Sorcery access, the skill
  hard cap, both skill-cost formulas, all controller masks, and confirmed
  Karma/stat writes.
- **Qt screen behavior covered:** all 13 profile headers, all 260 visible
  profile/cell combinations, their numeric glyphs and source offsets, every
  cursor cell, the command view's read-only exit, and clean field return.
- **Independent visual evidence:** one longplay Attributes/Skills spending
  frame matches the Human/Shaman header, the ROM-backed skill labels and
  description, and the Throwing rank-0 to rank-1 cost preview. Its differing
  live attribute/Karma values are not linked to a RAM capture.
- **Still partial visually:** the independent video does not cover all class
  and race presets, cursor permutations, rejected purchases, or the view-only
  command-menu variant. Other routines that consume attribute/skill values
  need their own audits.

See [the Karma page distinction audit](karma-status-and-hotel-upgrade.md),
[the archetype profile audit](archetype-profile-permutation-audit-2026-10-23.md),
and ROM entry `0xD82A` in `decompiled-function-status.tsv`.
