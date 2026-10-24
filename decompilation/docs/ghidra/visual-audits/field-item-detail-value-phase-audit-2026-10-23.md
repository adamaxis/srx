# Field inventory detail-value phase audit

## Finding

The selected item detail number is the selected inventory cell's phase byte at
runner record `+0x6E + slot`, indexed into the 40-word table at ROM `0x6AC46`.
`0x12A8C` selects and draws that value. The host previously had no source-correct
numeric asset for this display, so selected Medkit pages omitted `uses 06` and
`uses 05`.

The glyph source is the raw counter tile upload at `0x334C`: the ROM transfers
`0x4C0` bytes from `0x6A5BE` to VRAM `0xDAC0`. This differs from the unrelated
command-tab asset stream previously used for the detail values. The usable table
has 40 rows (`0..39`), matching the largest occupied eligible profile phase
observed here (`0x25`, decimal 37).

## ROM and profile coverage

- ROM first: `0x334C` tile upload, `0x12A8C` selected-row lookup, `0x6AC46`
  value table, live runner `+0x66..+0x6D` item codes and `+0x6E..+0x75` phases.
- Thirteen source records at `0x7291A..0x7351A` are checked byte-for-byte by
  the physical profile QA. Its 43 selected states cover every occupied eligible
  item entry and phases 0 through 37. The selected detail values are checked
  against named assets in full-screen output; Debug and Release produced the
  same 43 relative files and SHA-256 hashes.
- The focused host First Aid route shows Medkit uses 06 before treatment and 05
  after treatment, matching the named number assets. The independent longplay
  contains a longer Joshua Medkit segment, detailed below; the host route and
  footage are separate evidence.
- Item detail phase `+0x6E` is separate from live ammunition `+0x4E`. The field
  HUD derives active weapon count using its equipped code's D4540 `+0x24`
  capacity; this audit does not infer ammo from the selected detail value.

## Validation artifacts

- `build/conversion/qt/{debug,release}/qt_runtime/field-profile-combat-bars/detail-value-profile-permutation/`
  contains `profile-detail-value-cases.tsv` and 43 full-screen BMPs in each
  configuration.
- `build/conversion/qt/{debug,release}/qt_runtime/field-first-aid-qa/`
  contains `first_aid_target.bmp` and `first_aid_medkit_used.bmp`.
- Focused Debug and Release CTests passed:
  `shadowrun_ghidra_conversion_qt_field_first_aid_qa` and
  `shadowrun_ghidra_conversion_qt_field_profile_combat_bar_permutation_qa`.
- The First Aid capture hashes matched between configurations:
  Target 06 `DB90A010ED20B7DBD4C3E56CB48A6118D4A305DD9B98B042D42CEA9FA0EFBA51`;
  used 05 `F97198DC944D95785BCFAEFEEB40217734B32379995B30F715687F033015BF09`.

## Independent longplay count sequence

I reopened full-resolution frames from the supplied section 06/07 captures.
They show the selected Joshua Medkit count progressing through these sampled
states; section 06 frame 002563 also shows a separate Stim Patch at 04.

| Capture | Visible UI state | Medkit uses |
| --- | --- | ---: |
| Section 06, `002580` | First Aid menu | 06 |
| Section 06, `002610` | `Heal who?` target list | 05 |
| Section 06, `002640` | `Medkit used.` result | 04 |
| Section 06, `002655` | First Aid menu | 03 |
| Section 06, `002685` | `Heal who?` target list | 02 |
| Section 06, `002699` | Inventory menu | 01 |
| Section 07, `000030` | `Medkit used.` result | 01 |
| Section 07, `000120` | Field view after leaving the menu | -- |

This is direct evidence that the displayed item detail tracks the changing
stack state across multiple menu redraws and a section boundary. The sampled
frames do not log every controller event, so they do not establish an exact
one-frame-per-use input chronology or show the final zero-count inventory cell.

The ROM path connects the display to that state. `0x1243C` branches by selected
item code. Medkit code `0x10` calls `0x1253E`; the fully treated target guard
returns without consuming a use. After a treatment, `0x124DC` decrements the
selected slot's `+0x6E` phase byte. A positive result keeps the stack; zero
calls `0x1275C` to clear the slot and `0x11624` to redraw equipment highlights.
The selected-detail renderer at `0x12A8C` reads the same slot phase for the
visible count (`0x12BE8`). Raw ROM bytes confirm the decrement at `0x124DC`
(`53 30 70 6E`) and the slot clears at `0x127A6/0x127AA` (`42 30 70 66` /
`42 30 70 6E`). The sibling First Aid codes are `0x11` (mental patch) and
`0x12` (physical patch); `0x13` follows the source automatic-use branch.

During this audit, the C++ First Aid helper and its unit fixture incorrectly
accepted `0x13` as a physical patch. The item classifier routes `0x13` to
automatic use, and Lua's First Aid rule already limits the valid range to
`0x10..0x12`. The helper and fixture are now aligned with those source-backed
branches. This was a helper/test contract defect; the controller path already
filtered `0x13` before calling the helper, and no independent screenshot shows
that item being misused as First Aid. The conversion unit case now checks
phase 2 -> 1 -> cleared on a final successful use and verifies a fully healed
target preserves the stack without rolling. `shadowrun_ghidra_first_aid_conversion`
and `shadowrun_ghidra_conversion_qt_field_first_aid_qa` pass in Debug and
Release after rebuilding the Qt QA runner. The physical screenshot route still
covers one use; a controller capture of final depletion and no-target behavior
remains open.

## Remaining boundary

This is a source-backed numeric selection and decrement audit, not a defended
inventory hill. The independent longplay shows one Joshua Medkit stack and a
Stim Patch detail, but not weapon switch/reload, the Medkit's final cleared-cell
frame, or other race/class profiles. The host profile captures verify the
ROM-selected assets and state plumbing but are not independent Genesis fidelity
evidence. The former `detail-value-40..255.bmp` files remain on disk; they are
absent from the active catalog/resource map and are not loaded.

Next audit: capture the final Medkit depletion and no-target branches through
the controller, including the phase-1 clear, full-target no-consumption case,
and clean return. Continue searching the supplied video for independent weapon
switch, reload, and post-switch Atk/Def evidence.
