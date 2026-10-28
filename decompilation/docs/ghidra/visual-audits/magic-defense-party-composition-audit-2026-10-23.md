# Magic Defense Party Composition Audit

Date: 2026-10-23

Scope: the Physical and Mana Defense values on the Magic UI, with emphasis on
party composition, source runner presets, focus placement, and numeric strip
selection.

## ROM rule traced

The page refresh at `0x1055E` enters `0x1047E` for Defense. That routine scans
exactly three party records. A record contributes only when its ready bit at
`+0x10` is set and its class byte at `+0x9B` is 0 (Mage) or 1 (Shaman). The
contribution comes from `0x55F02`: Sorcery at `+0x80` plus the phase byte for
the first Power Focus (`0x18`) found while scanning inventory slots 6 down to
0. Slot 7 is outside that scan. The low-byte addition wraps before the value
is divided by four; the remainder rounds upward only at 3.

The party contributions are added to the active runner's Body (`+0x77`) for
Physical Defense and Willpower (`+0x7C`) for Mana Defense. These are byte
additions. The value selects row 0 when it is zero, otherwise row `value - 1`.
The spell panel writes Physical at `0xC808` and Mana at `0xC908`; the source
skips the Mana write when its value is zero. The non-spell fallback uses the
separate typed plans at `0xC51E` and `0xC69E`.

Race has no direct branch in this formula, but race/class presets change the
recipient attributes and the pool's eligibility and Sorcery inputs. The raw
US ROM records used by this audit have these different inputs:

| Record | Race / class | Body | Willpower | Sorcery | Can contribute to party pool |
|---|---|---:|---:|---:|---|
| Joshua Samurai `0x7291A` | Human / Samurai | 4 | 4 | 0 | No |
| Joshua Decker `0x72A1A` | Human / Decker | 2 | 3 | 0 | No |
| Joshua Gator Shaman `0x72B1A` | Human / Shaman | 2 | 4 | 3 | Yes |
| Rat Shaman `0x72C1A` | Dwarf / Shaman | 4 | 4 | 3 | Yes |
| Troll Samurai `0x72D1A` | Troll / Samurai | 7 | 3 | 0 | No |
| Human Mage `0x72E1A` | Human / Mage | 2 | 4 | 3 | Yes |
| Dwarf Decker `0x72F1A` | Dwarf / Decker | 4 | 3 | 0 | No |
| Bear Shaman `0x7301A` | Orc / Shaman | 6 | 5 | 4 | Yes |
| Elf Decker `0x7311A` | Elf / Decker | 3 | 4 | 0 | No |
| Human Samurai `0x7321A` | Human / Samurai | 5 | 4 | 0 | No |
| Elf Mage `0x7331A` | Elf / Mage | 3 | 5 | 5 | Yes |
| Elf Decker `0x7341A` | Elf / Decker | 1 | 6 | 0 | No |
| Human Samurai `0x7351A` | Human / Samurai | 5 | 5 | 0 | No |

The wider [attributes and skills profile audit](attributes-skills-profile-permutation-audit-2026-10-23.md)
records the complete preset rows and the race-specific maximum table. The
[Magic spell/action audit](magic-spell-action-permutation-audit-2026-10-23.md)
records each caster's spell table and its individual minimum/maximum Force
range; those bounds are separate from this Defense pool.

The attribute/skill audit traces Sorcery training for Mage/Shaman from rank 0
through 12 and rejects rank 13; its ROM race table caps Body at 11 for Troll
and Willpower at 7 for Dwarf. The active-record selector at `0x0397A`/`0x03998`
sets and advances `FFFC04` through `0xFF0100`, `0xFF0200`, and `0xFF0300`,
the same three records scanned by `0x1047E`. Gregory's Power Focus service at
`0x22C46` writes item `0x18` at phase 2. For the 13 source profiles in this
audit, the phase-2 bound is 3 per ready caster (`(12 + 2) / 4`) and a
three-caster pool of 9. Profile compatibility matters: the Troll profile is
Samurai, so it cannot supply that pool. The maximum Physical value across those class/race
ceilings is 17: either the Orc Bear Shaman at Body 8 plus pool 9, or the Troll
Samurai at Body 11 plus two other casters contributing 6. The Dwarf Rat Shaman
can reach Willpower 7 while remaining an eligible caster, giving a Mana bound
of 16 with pool 9.

The raw ROM also contains an Agira-adjacent offer prompt at `0x10D708`. Under
the shared prompt layout at `0x1F3C4`, its three labels are at `0x104234`,
`0x104256`, and `0x105101`; command slots `0xEB` and `0xEC` are at `0x10D71C`
and `0x10D720`. The dispatch table at `0x12CCB4` maps these commands to
`0x24A1E` and `0x24A56`. Command `0xEC` checks that the active class is Mage or
Shaman, requires a free inventory slot and 80,000 nuyen, then writes item
`0x18` with phase 4. It therefore yields a per-caster contribution of 4 at
Sorcery 12. If all three ready party records can legally acquire or hold that
phase, the conditional bounds are Physical 20 (Orc Bear Shaman Body 8 plus
pool 12) and Mana 19 (Dwarf Rat Shaman Willpower 7 plus pool 12). The Troll
Samurai case has only two other party records to contribute, for Physical 19.

This is not yet a confirmed Agira purchase route. The Vid-Phone callback at
`0x22CE8` returns prompts `0x10D698`, `0x10D6A0`, or `0x10D6A8`; accepted
command `0x7F` at `0x22D18` returns `0x10D6C0`. None directly selects
`0x10D708`, and no aligned literal pointer to that prompt was found in the
ROM. A computed or shared continuation remains possible, so the phase-4
scenario is conditional. Party permutations in the CTest remain arithmetic
inputs rather than proof of recruitment reachability, and other item-phase or
restored-save writers have not been traced. The byte sweep below stays separate
from these profile and acquisition bounds.

## Permutation coverage

`magic_defense_party_permutation_conversion_test.cpp` loads all 13 profiles
from the raw US ROM and asserts each record's race/class identity. Its oracle
is separate from the production calculation. It checks:

- all `26^3 * 13 = 228,488` ordered party inputs formed from each profile's
  ready and not-ready state, for every profile as the active Body/Willpower
  recipient;
- all eight inventory positions for a single inserted Power Focus on every
  profile, including slot 7's excluded behavior;
- every pair of Power Focus positions on every profile, including slot 7
  competing with each scanned position;
- all 65,536 Sorcery-byte/phase-byte pairs on one eligible source profile,
  covering every wrap and quarter-rounding boundary;
- both the spell-panel refresh and fallback plan, including value, row,
  destination, and the spell panel's zero-Mana omission rule.

Party combinations in this test are arithmetic permutations of the three
source records. They do not claim that every repeated-record permutation is
reachable through the recruitment UI. The byte-domain sweep verifies machine
arithmetic; it does not claim all byte values are legal Sorcery or item
ratings. Reachable progression limits need their own source trace. The
existing `field-magic-qa` covers page-5 A entry, hold, and fresh-edge return;
the [Magic spell/action audit](magic-spell-action-permutation-audit-2026-10-23.md)
covers the spell, force, and posture controller masks. This matrix adds the
numeric state permutations across those already-audited screen paths.

## Screenshot check and limits

The Debug `field-magic-nonmagic-qa/actual.bmp` host capture was reviewed at
320x224. Its Joshua Samurai preset has Body 4 and Willpower 4, no eligible
party caster contribution, and shows the same row 3 bar for both strips. The
route QA verifies the source-mapped `+0x10` byte, recomputes both values from
the party records, checks the selected `defense-bar-N.bmp` assets at the
traced positions, holds the view for 30 frames, and returns on a fresh input
edge. This is an internal renderer check, not an independent Genesis capture.

The five Debug spell-panel profile captures were also reviewed. Their
source-selected entries differ as expected: Rat Shaman / Stink, Human Mage /
Flame Dart, Bear Shaman / Mana Zap, Elf Mage / Mana Blast, and Joshua Gator
Shaman / Mana Zap. The spell/action audit ties each table entry to its source
force maximum and the presenter; these profile frames confirm the host view
uses the selected class table and matching spell art, but remain internal
screenshots.

The supplied longplay sections 01-07 contain no matching non-spell Magical
Defense frame. The independent section-06 captures support Gator Shaman spell
views but do not expose a party-composition Defense permutation. The audit
therefore verifies the ROM-derived calculation and host strip selection, but
does not claim Genesis visual fidelity for every preset or party mix. The Magic
hill remains partial.

## Verification

Debug and Release CTest `shadowrun_ghidra_magic_defense_party_permutation`
pass. Each run reports 228,488 party combinations, 104 single Focus positions,
364 overlapping Focus pairs, and 65,536 byte-domain cases. The test also
checks the typed plans for both Magic Defense presentation paths; it does not
produce an independent visual golden.
