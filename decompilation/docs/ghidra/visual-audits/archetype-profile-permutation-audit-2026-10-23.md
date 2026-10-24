# Joshua Archetype Profile and Input Audit

Date: 2026-10-23  
ROM: US Genesis Shadowrun  
Scope: New Game archetype menu, all three starter records, resource preview, and controller-byte permutations

## ROM path

The New Game selection in `0x59092` enters the archetype screen at `0x59248`.
That routine draws the three choices and title, starts on Samurai, and uses
`0xC376` to place the selector. The input loop checks Up (`0x59282`) before
Down (`0x59294`). Either direction updates the selected resource/portrait
preview through `0x1F38A` and `0x1F34A`, then returns to the loop without
processing a confirmation from the same sample. A, C, or B confirms; Start is
not read in this loop.

At `0x59308`, the selected byte is copied to D0 and `0x42B0` copies 64
longwords (one full `0x100`-byte runner record) from the table beginning at
`0x7291A`. It also writes the three source selection markers. The screen
offers exactly three starter records; those are the profiles audited below.

## Starter profiles read from the ROM

The raw US ROM bytes were read at `0x7291A + index * 0x100`. Attributes use
`+0x77..+0x7C`; skills use `+0x80..+0x85` and `+0x87..+0x8C`; race and class
use `+0x9A/+0x9B`; Magic uses `+0x7F`; and the spell table word is `+0xA0`.
The skill labels follow the source Attributes/Skills catalog.

| Choice / table index | Race / class | Body, Quickness, Strength, Charisma, Intelligence, Willpower | Sorcery, Firearms, Pistols, SMGs, Shotguns, Melee, Throwing, Computer, Biotech, Electronics, Reputation, Negotiation | Magic / spell table |
|---|---|---|---|---|
| Samurai / 0 | Human 0 / Samurai 2 | 4, 3, 4, 1, 2, 4 | 0, 4, 0, 0, 0, 3, 2, 0, 1, 0, 1, 1 | 0 / 0 |
| Decker / 1 | Human 0 / Decker 3 | 2, 5, 2, 2, 4, 3 | 0, 2, 0, 0, 0, 1, 0, 3, 2, 1, 1, 2 | 0 / 0 |
| Gator Shaman / 2 | Human 0 / Shaman 1 | 2, 4, 1, 2, 3, 4 | 3, 2, 0, 0, 0, 1, 0, 0, 0, 0, 1, 3 | 6 / 4 |

These presets are not interchangeable. The Shaman begins with Magic and
Sorcery and uses spell table 4; Samurai and Decker have no Magic resource,
and the ROM class gate marks their Sorcery cell restricted. Samurai starts
with stronger Firearms/Melee/Throwing values, while Decker starts with
Computer/Biotech/Electronics values. The archetype screen itself shows the
choice label and portrait, not the numeric profile; the selected record drives
later menus and game rules.

All three starter choices have Human race index 0. The six raisable attribute
ceilings therefore use the Human row `6, 6, 6, 6, 6, 6` from the ROM maximum
table at `0xC22A6`. Recruited templates add other class/race combinations and
different ceilings, documented with their source values in the [Magic
profile audit](magic-spell-action-permutation-audit-2026-10-23.md). No extra
class/race combinations are inferred from the three-choice startup menu.

## Input permutations

The Debug/Release startup integration test now applies all 256 possible
controller-byte masks from each of the three selected rows (768 cases). It
checks that Up wins over Down, direction wins over A/C/B, wrapped movement
redraws the cursor and resource preview exactly once, confirmation runs only
without a direction, and Start/unused bits alone do nothing. It also verifies
that each selected starter path copies the corresponding complete ROM record
to the live Joshua record.

## Visual comparison and disposition

Independent longplay section 02 frames `000050` through `000062` stay on the
Gator/Shaman choice. Frame `000060` shows the Gator portrait, three labels,
selected arrow, and device shell. I inspected it beside the Qt
`10_archetype_gator_shaman.bmp`; portrait, row placement, selector, and shell
match visually. Qt QA also writes `08_archetype_samurai.bmp` and
`09_archetype_decker.bmp`, but the supplied video has no matching independent
selection states for those two choices. Their profile/record transfer is
source-checked; those images are not Genesis visual proof.

- **Code changed:** startup integration QA now sweeps the 768 masks and checks all three selected record copies.
- **ROM behavior implemented:** three-choice order, direction/confirmation precedence, preview refresh, and `0x42B0` record transfer are represented by typed menu state and callbacks.
- **Independently visually checked:** Gator/Shaman selection at section 02 frame `000060`.
- **Still partial:** no independent Samurai or Decker selection frame is available; the QA screenshots for those states are internal artifacts.

See the archetype rows in [the Qt visual audit index](../visual-analysis/qt-visual-audit.tsv) and ROM entries `0x59092`, `0x59248`, `0x59308`, and `0x42B0` in `decompiled-function-status.tsv`.
