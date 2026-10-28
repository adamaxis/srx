# Hotel Karma Upgrade Profile and Boundary Audit

Date: 2026-10-23  
ROM: US Genesis Shadowrun  
Scope: Hotel Karma page across source runner presets, race ceilings, class
restrictions, preview arithmetic, purchase inputs, and resulting pixels

## ROM inputs and display rules

Hotel Options selects the spending route at `0xC14C8 -> 0xD024`. The command
Attributes/Skills view instead writes the negative view-only marker before
entering the shared `0xD82A` grid. The Hotel child reads the selected live
runner record and keeps the parent Hotel menu suspended until its return path
at `0xD4A0`.

The six raisable attributes are stored at `+0x77..+0x7C`. Race index `+0x9A`
selects six maximum bytes from the raw ROM table at `0xC22A6` (five races by six
attributes). The next value is `current + 1`; attribute Karma cost equals that
next value. There is no lower-bound rejection in the source: a zero attribute
can rise to one for one Karma. Essence `+0x7D` and Magic display `+0x7E` are
not upgradeable by this page. The focused Essence detail reads `+0x7E` and
`+0x8E`, while focused Magic reads its resource value at `+0x7F`.

Skills are stored at `+0x80..+0x85` and `+0x87..+0x8C`; `+0x86` is skipped.
Class index `+0x9B` blocks Sorcery `+0x80` when it is 2 or greater (Samurai and
Decker). Mage and Shaman can train Sorcery. Skill ranks stop when the candidate
next rank reaches 13. Ordinary skills cost twice the next rank. The left
column's Pistol, SMG, and Shotgun cells use `ceil(1.5 * next rank)`; if the
selected skill is zero, its next rank uses Firearms `+0x81` as its base first.
The active record's Karma balance is `+0x8D`. `0xD3EA` checks funds and writes
both the selected record field and `+0x8D` only after the `0xD53C` confirmation
accepts.

These rules consume each profile's own bytes. A class label does not synthesize
stats or skills, and the displayed numbers are not generic class defaults.

## Source profiles and Body preview

The QA reads the three Joshua archetypes at `0x7291A`, `0x72A1A`, and `0x72B1A`,
then the ten named recruits from `0x72C1A..0x7351A`. For all thirteen profiles,
it compares the complete named 0x100-byte asset with the raw US ROM record
before decoding it into the live typed runner record. The table below shows
how the same Body preview varies with the source preset and race ceiling.

| Profile | Race / class | Body source value | Body ceiling | Preview | Cost |
|---|---|---:|---:|---:|---:|
| Joshua Samurai | Human / Samurai | 4 | 6 | 5 | 5 |
| Joshua Decker | Human / Decker | 2 | 6 | 3 | 3 |
| Joshua Gator Shaman | Human / Shaman | 2 | 6 | 3 | 3 |
| Rat Shaman | Dwarf / Shaman | 4 | 7 | 5 | 5 |
| Troll Samurai | Troll / Samurai | 7 | 11 | 8 | 8 |
| Human Mage | Human / Mage | 2 | 6 | 3 | 3 |
| Dwarf Decker | Dwarf / Decker | 4 | 7 | 5 | 5 |
| Bear Shaman | Orc / Shaman | 6 | 9 | 7 | 7 |
| Elf Decker 1 | Elf / Decker | 3 | 6 | 4 | 4 |
| Human Samurai 1 | Human / Samurai | 5 | 6 | 6 | 6 |
| Elf Mage | Elf / Mage | 3 | 6 | 4 | 4 |
| Elf Decker 2 | Elf / Decker | 1 | 6 | 2 | 2 |
| Human Samurai 2 | Human / Samurai | 5 | 6 | 6 | 6 |

The full grid sweep covers the different source values in every row, including
current Karma. The existing [Attributes/Skills preset audit](attributes-skills-profile-permutation-audit-2026-10-23.md)
records the remaining attribute, skill, magic-resource, and spell-table values.

## Physical Hotel permutation

`shadowrun_ghidra_conversion_qt_field_hotel_profile_permutation_qa` enters
Hotel Karma from the ordinary Hotel Options page for every raw profile. It
checks the black loading phase and then walks all twenty grid cells with real
controller d-pad edges. At every cursor position the QA verifies the selected
typed cell and source record offset, independent ROM-derived preview status,
next value and cost, race/class header, current `+0x8D` Karma pixels, unselected
grid labels/numbers, selected value detail, and the lower preview/cost glyphs.
Focused Essence and Magic are checked against their special `+0x7E/+0x8E` and
`+0x7F` detail sources. This is 260 profile/cursor states.

The UI boundary frames additionally cover:

- All six attributes at their ROM ceiling for one source profile of each race
  (30 race/attribute cases), including the selected maximum-level message and
  an A press that leaves the capped value unchanged.
- Rank-12 Firearms for each of the four source classes and rank-12 Sorcery for
  Mage and Shaman; Samurai and Decker each exercise the Mage Only A-action.
- A zero Charisma candidate previewing one for cost one, showing that no
  minimum clamp blocks an increase from zero.
- Zero Firearms plus zero Pistols, where the reduced-cost path previews one
  for cost two.

For each profile, the physical transaction sequence sets Karma to one below
the ROM-derived Body cost and verifies the insufficient message leaves Body
and Karma unchanged. It then sets exact funds, uses B to open the default-Yes
prompt, moves Down to No and presses B to decline without mutation, reopens
with A, and confirms with A. The QA checks Body `+0x77`, Karma `+0x8D`, the
visible current-value glyph, and the recalculated next preview. It follows
with Start back to Hotel Options. One Human Samurai purchase reaches Body 6
and immediately displays the Human Body maximum instead of another preview.

The output `profile-cells.tsv` records the raw source record address, cursor
column/row, record offset, current value, status, next value, cost, Karma, and
relative BMP path. Each profile has a full-screen `actual.bmp`, all twenty
cell frames, the relevant boundary states, transaction frames, and a Hotel
return frame. The QA also presses Left/Up at the upper-left cell and Right/Down
at the lower-right cell for every profile; each input must preserve its cursor,
preview, runner fields, status, and full-screen pixels. The successful purchase
holds A for four frames and must apply exactly one upgrade, matching the input
duration in the supplied Karma capture. Debug and Release each write 492 BMPs;
all relative paths and SHA-256 hashes match. `physical-route.tsv` in the paired
field-Hotel output retains the ordinary Stoker entry, Hotel service, and
existing checkout path.

## Visual review and limits

I reviewed full 320x224 captures for Joshua Samurai's normal Body preview and
Mage Only response, Troll Samurai's higher Body cost, Human Samurai's
post-purchase maximum, and the zero-rank reduced-cost case. The current host
presenter draws the Mage Only/insufficient message in the same y=176..184 area
as the selected cell description; the artifacts preserve that composition.
No matching independent Genesis Hotel Karma screen was identified in the
supplied longplay, so the capture cannot establish whether that overlap is
pixel-faithful.

- **ROM behavior traced:** `0xC14C8`, `0xD024`, `0xD82A`, `0xD3EA`, `0xD53C`,
  and `0xD4A0`; record inputs, race table, class gate, skill costs, confirmation,
  writes, and exit.
- **Host behavior verified:** all thirteen raw profiles, 260 profile/cell
  presentations, 30 race ceilings, class and rank boundaries, zero boundaries,
  insufficient funds, B/A confirmation requests, No, Yes, Karma/stat updates,
  and Start returns.
- **Independent visual verification:** unavailable for the exact Hotel Karma
  page. Keep the visual hill partial until a matching Genesis capture is found.

