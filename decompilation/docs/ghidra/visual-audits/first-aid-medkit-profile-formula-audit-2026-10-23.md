# First Aid Medkit Profile Formula Audit

Date: 2026-10-23  
ROM: US Genesis Shadowrun  
Scope: Medkit roll inputs, target difficulty, treatment result, profile
permutations, and the visible item-count connection.

## ROM path and inputs

The Medkit leaf at ROM `0x1253E` reads the acting runner's Biotech byte from
`+0x89` into the trial count at `0x12544`. It reads the selected target's
physical meter `+0x4C` and mental meter `+0x9C`; when both equal ten it exits
before the random service. Otherwise physical meter bands choose base
difficulty 8 for 0..3, 6 for 4..6, and 4 for 7..10 (`0x12572..0x12590`). The
target's Body at `+0x77` is divided by three and subtracted, with a floor of
two (`0x12592..0x125AE`). The Medkit passes Biotech and this target difficulty
to `0x0DBA` at `0x125B2`.

`0x0DBA` treats a nonpositive trial count as one trial. Each trial rolls a
six-sided result, accumulating sixes until a non-six face, then counts a
success when the accumulated result meets the difficulty. The Medkit converts
zero successes to one point of treatment (`0x125C0..0x125C8`); positive
successes are used as-is. The physical and mental services then clamp the
target meters to ten, apply the source active-status behavior, clear status
bits 6 and 4, set bit 0, and install continuation `0xB45C` (`0x125CA..0x125E8`).
The parent item loop decrements the selected phase only after success at
`0x124DC`; a phase reaching zero clears the cell through `0x1275C`.

This separates healer from target inputs. Healer race/class do not have a
direct branch in the Medkit leaf: the active runner's profile contributes
Biotech `+0x89`. Target race/class also do not have a direct branch there: the
target contributes physical meter, mental meter, and Body. Race-specific Body
ceilings constrain available Body values through the table at `0xC22A6`.
Target mental meter does not change difficulty; it participates in the fully
treated guard and receives the same treatment delta as physical meter.

## Source profile inputs

These values were read from the raw US ROM at the 13 Joshua/recruited runner
records. Race maximums are the Body column at `0xC22A6`.

| Profile | Record | Race / class IDs | Starting Body / race maximum | Biotech `+0x89` |
|---|---:|---:|---:|---:|
| Joshua Samurai | `0x7291A` | Human 0 / Samurai 2 | 4 / 6 | 1 |
| Joshua Decker | `0x72A1A` | Human 0 / Decker 3 | 2 / 6 | 2 |
| Joshua Gator Shaman | `0x72B1A` | Human 0 / Shaman 1 | 2 / 6 | 0 |
| Rat Shaman | `0x72C1A` | Dwarf 2 / Shaman 1 | 4 / 7 | 0 |
| Troll Samurai | `0x72D1A` | Troll 4 / Samurai 2 | 7 / 11 | 1 |
| Human Mage | `0x72E1A` | Human 0 / Mage 0 | 2 / 6 | 0 |
| Dwarf Decker | `0x72F1A` | Dwarf 2 / Decker 3 | 4 / 7 | 0 |
| Bear Shaman | `0x7301A` | Orc 3 / Shaman 1 | 6 / 9 | 0 |
| Elf Decker | `0x7311A` | Elf 1 / Decker 3 | 3 / 6 | 2 |
| Human Samurai | `0x7321A` | Human 0 / Samurai 2 | 5 / 6 | 3 |
| Elf Mage | `0x7331A` | Elf 1 / Mage 0 | 3 / 6 | 0 |
| Elf Decker | `0x7341A` | Elf 1 / Decker 3 | 1 / 6 | 3 |
| Human Samurai | `0x7351A` | Human 0 / Samurai 2 | 5 / 6 | 5 |

This gives five distinct Biotech ranks (0, 1, 2, 3, 5) across all five races
and four classes. The rank-0 records still pass zero to the source roll helper;
the helper's one-trial minimum is a separate rule from a rank-1 profile.

## Numeric and visual behavior

The First Aid detail number is the selected cell's phase/use count, not the
number of Medkit successes or the amount of health restored. It reads the
same item phase consumed after a successful treatment; see the
[item-detail phase audit](field-item-detail-value-phase-audit-2026-10-23.md).
The visible route is Inventory -> First Aid -> target selection -> `Medkit
used.` -> Inventory. A fully treated Medkit target returns the source text key
`items.text.option.first_aid_not_injured` (`This person is` / ` not injured!`)
without rolling or consuming a use. The actual treatment amount changes the
target's physical/mental meters and status; it is not printed as a numeric
healing value. The selected item detail is its phase/use count, so it stays at
`01` on this guard.

The independent section 06/07 longplay supports the visible Gator/Shaman
Medkit sequence and the stack-count redraws across First Aid, target, result,
Inventory, and field return. This is one Joshua preset and one observed
sequence. It does not establish the exact random result, controller chronology
for every sampled frame, or visuals for all thirteen profiles.

## Regression matrix and code connection

`shadowrun_ghidra_first_aid_conversion` now reads all 13 profile identities,
Biotech bytes, Body bytes, and race Body ceilings directly from the US ROM. For
each source healer/target profile pair, it sweeps every legal Body value for
the target race, all 121 physical/mental meter pairs, and every success count
from zero through the healer's legal trial count (with the source one-trial
minimum for Biotech zero). It checks the exact roll request, the three
physical-difficulty bands, Body `/3` reduction and minimum difficulty, zero
success minimum treatment, meter caps, status bits, and `0xB45C` continuation.
It also checks 1,313 fully treated cases do not call RNG or mutate target
state. Total non-full treatment cases: 436,320.

Debug and Release CTests pass. The Qt physical-input QA now routes all thirteen
raw source profiles through Inventory, First Aid target selection, a Medkit
use, result acknowledgement, and an already-full target attempt. It checks
profile-derived race/class, Body and Biotech; expected dice samples and meter
results; then the full-target message, zero die samples, unchanged meters,
status, continuation, item, and phase. For the full-target attempt it seeds a
one-use Medkit after returning to Inventory and reselects that cell with
controller input. Troll Samurai uses its legal Body ceiling 11, physical 8,
and difficulty 2. The QA writes five profile frames per record (65 BMPs); all
relative filenames and SHA-256 hashes match between Debug and Release. These
are host controller-route captures, not independent Genesis screenshots.

The runtime connection currently passes the active runner's `skills[8]` and
the target's Body/meters from `field_scene_runtime.cpp` into the typed helper
and shared random service. A parallel `first_aid.lua` rule exists; this audit
does not establish that Lua is the authoritative First Aid state machine, so
the project architecture boundary remains open. The independent longplay
still supports only the observed Joshua Gator/Shaman use sequence; it has no
profile-wide or full-target message sequence.

Keep the First Aid and inventory hills partial. No independent profile-specific
Genesis screenshots or route-level Windows audio loopback have been added.
