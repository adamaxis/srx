# First Aid Patch Profile Permutation Audit

Date: 2026-10-23  
ROM: US Genesis Shadowrun  
Scope: Stim Patch (`0x11`) and Trauma Patch (`0x12`) effect branches, target
guards, status bits, phase consumption, profile inventory placement, and
visible result text.

## ROM behavior

The inventory handler reads the selected item code from its live record slot
at `0x1243C`. The item-code split at `0x12440` sends Medkit `0x10` to the
separate roll-based handler; Stim Patch `0x11` enters `0x1247E`, while Trauma
Patch `0x12` follows `0x1244C`/`0x12466`.

- **Stim Patch (`0x11`):** target mental meter `+0x9C == 10` branches to the
  source no-injury text at `0x12456`. Otherwise `0x37EC` advances mental by 2.
  The handler then checks target physical meter `+0x4C`; when nonzero it clears
  status bits 6 and 4 at record `+0x10` and sets bit 0.
- **Trauma Patch (`0x12`):** target physical meter `+0x4C == 10` branches to the
  same no-injury text. Otherwise `0x3674` advances physical by 2. The handler
  checks target mental meter `+0x9C`; when nonzero it clears status bits 6 and
  4 and sets bit 0.
- **Shared success tail:** both branches join at `0x124AC`, decrement selected
  phase `+0x6E` at `0x124AE`, and clear a depleted item cell through
  `0x1275C`. These two patch branches do not call the random service. Successful
  patches display the source key `items.text.option.first_aid_slap_patch_used`
  (`Slap-patch used.`); a full-side guard displays
  `items.text.option.first_aid_not_injured` (`This person is` / ` not injured!`)
  and leaves the phase unchanged.

The patch formula has no direct race, class, Body, or Biotech operand. Profile
records still matter for their item-code/phase layout and the target's live
meters/status. The Medkit's Body/Biotech formula and all-profile record table
are in the [Medkit formula audit](first-aid-medkit-profile-formula-audit-2026-10-23.md).

Ghidra's `fn_00011740.cpp` cross-checks the raw instruction flow at
`0x1243C..0x124AE`; ROM bytes remain the primary source.

## Profile and action coverage

`shadowrun_ghidra_conversion_qt_field_first_aid_qa` now runs four physical
controller routes for every one of the thirteen Joshua/recruited source
profiles: Stim success and full-mental guard, plus Trauma success and
full-physical guard. It verifies the action menu, target row, source message,
live HUD meters, unchanged Body/Biotech, status branch, continuation, dice
count, phase, and Inventory return.

Successful test inputs alternate the source status edge cases by profile:

| Item | Opposite meter zero | Opposite meter nonzero | Guard |
|---|---|---|---|
| Stim Patch | Physical 0, mental 8 -> 10; status `0x51` retained | Physical 3, mental 6 -> 8; status `0x51` -> `0x01` | Physical 5, mental 10; no change |
| Trauma Patch | Physical 8 -> 10, mental 0; status `0x51` retained | Physical 6 -> 8, mental 3; status `0x51` -> `0x01` | Physical 10, mental 5; no change |

For profiles that begin with the selected patch item, QA retains its source
cell and phase. Otherwise it seeds the patch in the already selected Medkit
cell, using phase 2. Examples retained from source templates include Gator
Shaman's Stim/Trauma phases 6 (`6 -> 5` on success) and Rat Shaman's phases 3
(`3 -> 2`). The patch manifest records the selected slot and whether its item
and phase both came directly from the profile. All 52 patch cases report zero
random-service events.

The QA writes 78 patch-specific BMPs: one selected-item frame per patch type
and profile, and both success/no-injury result frames per patch type and
profile. Together with the Medkit route, there are 143 profile BMPs. The
52-row patch manifest, 13-row Medkit manifest, relative image filenames, and
SHA-256 hashes match across Debug and Release.

## Visual reference and limits

Independent section 06 frame `002563` shows Joshua with Stim Patch selected
and `uses 04`, supporting the source inventory shell, item label, icon, and
detail-value layout. The raw Gator Shaman starter template carries phase 6;
the recorded `04` is a different live inventory state, so it is not an exact
numeric screenshot match. The supplied longplay does not show either patch
effect, either patch's full-side no-injury guard, or Trauma Patch selection.
Host screenshots and controller tests therefore remain internal conversion
evidence. Keep the First Aid/inventory hills partial; the Qt C++ controller
and Lua `item_action.lua` still hold parallel First Aid state paths, and
route-specific Windows audio loopback remains open.
