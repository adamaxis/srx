# Inventory Selector and Equip/Unequip Review

Date: 2026-10-06
Primary source: US ROM `decompilation/input/shadowrun_genesis_us.bin`.
Qt route: `shadowrun_ghidra_conversion_qt_field_action_qa`.

## Source and state

- ROM `0x11506` updates the selected inventory coordinates; `0x11592` builds
  the cursor sprite from those coordinates. The selected slot is
  `secondary_page * 2 + primary_page` in the active runner's typed inventory.
- The cursor is a one-pixel `#C4BE27` outline on the 24x16 item image. It is
  separate from the equipment-cell treatment installed through `0x11624` /
  `0x11678`. Item artwork is drawn after that cell treatment so the item
  remains visible.
- Equip/unequip is handled by `0x12148` / `0x121E0`; the inventory row is
  rebuilt after the nested action at `0x11928`. The Qt runtime binds these
  operations to one typed runner record and rebuilds the field/menu frame.

## Eight-question review

1. **Does it look correct? Yes for the current selector.** The selected item
   perimeter measures exactly 76 `#C4BE27` pixels, the perimeter count for a
   24x16 one-pixel rectangle. No matching-color pixels fill the interior.
   Independent section 06 frames `002560`, `002580`, `002610`, and `002640`
   show the same slim outline around selected inventory items. The exact
   weapon-switch transition is absent from the independent recording.
2. **Does the look match behavioral state? Yes.** The outline follows the
   typed primary/secondary inventory coordinates. Pixel checks exercised
   cells 0 and 1; source-selected cells without inventory items do not get an
   item cursor. The equipped-slot marker is checked separately from the
   current cursor.
3. **Are the visuals correct? Yes for selector geometry and current cleanup.**
   Fresh Debug frames show a single edge around the item, with the icon
   visible. Unequipped weapon 1 and weapon 2 grids match freshly rendered
   baselines. Repeated equip/unequip returns remove the equipped-cell
   treatment without leaving an extra black box or stale icon.
4. **Is it animated? No.** The cursor stays fixed while the same cell is
   selected, including a 30-frame hold. D-pad input changes its coordinates;
   leaving the inventory page clears it. No cursor animation counter is
   present in the traced draw path.
5. **Is there a sound?** Accepted menu movement requests event `0x25`, which
   maps to resource 037 (2.134 seconds including its release tail). The item
   action dispatch requests event `0x24` (2.484 seconds including its release
   tail). The ROM cursor drawing routines themselves request no sound. Host
   audio QA checks decoded playback and player state; output loopback for this
   exact inventory visit is not recorded.
6. **Are the other visible parts of the screen also correct? Yes for these
   checkpoints.** I reviewed all 28 current 320x224 Debug action frames, not
   only the item crop. Inventory, runner portrait, right HUD, detail text,
   posture strip, action prompt, and Clips return all remain visible in their
   expected states. No child button or text is drawn over a control that
   should have been cleared.
7. **Do multiple frames support the interpretation? Yes, with a stated limit.**
   Independent section 06 frames `002560`, `002580`, `002610`, `002640`, and
   `002699` show inventory movement, selection persistence through First Aid,
   and return. Section 07 frame `000030` shows the result and `000120` shows
   the later field screen. These support the outline and clearing behavior,
   but not the precise weapon equip/unequip transition. Internal QA covers
   equip, cancel, selection, unequip, replacement, switch, reload, and return.
8. **Does the new UI replace the old UI cleanly? Yes.** No new page is added
   by this selector review. The Clips child replaces the center panel; cancel
   redraws the parent command page pixel-identically. The field HUD and
   posture strip persist as shared interface regions.

## Full-screen files reviewed

Debug files under
`build/archive/decompilation/build-qt-runtime-debug/qt_runtime/field-action-qa/`:

- `00_inventory_action_equip.bmp`, `01_inventory_action_cancel.bmp`,
  `02_inventory_action_reselected.bmp`, `03_inventory_action_unselected.bmp`,
  `04_inventory_action_reselected.bmp`.
- `05_fresh_unequipped_baseline.bmp`, `05_weapon_1_unequipped.bmp`,
  `05_weapon_1_unequipped_frame30.bmp`, `06_combat_bars_after_equip.bmp`,
  `07_discard_confirmation_no.bmp`, `08_discard_confirmation_yes.bmp`,
  `09_inventory_after_discard.bmp`.
- `10_weapon_2_equipped_at_50_percent.bmp`,
  `10_weapon_2_equipped_at_50_percent_frame30.bmp`,
  `11_fresh_weapon_2_unequipped_baseline.bmp`,
  `11_weapon_2_unequipped_at_50_percent.bmp`,
  `11_weapon_2_unequipped_at_50_percent_frame30.bmp`,
  `12_weapon_1_reloaded_from_50_percent.bmp`.
- `13_reload_confirmation_no.bmp`, `14_reload_confirmation_yes.bmp`,
  `15_weapon_1_reloaded_to_capacity.bmp`,
  `16_clips_closed_parent_redraw.bmp`,
  `17_clips_cancel_clears_child_controls.bmp`,
  `18_weapon_1_unequip_redraw_repeat.bmp`,
  `19_weapon_2_equipped_full_capacity.bmp`,
  `20_weapon_2_empty_magazine.bmp`,
  `21_weapon_2_reloaded_to_capacity.bmp`, `actual.bmp`.

The 28 matching Release BMPs have identical SHA-256 hashes. The five older
debug-only zoom/contact images were also inspected. `item-cell-zoom.bmp` is a
4x nearest-neighbor crop; its four display pixels per edge represent one
source pixel and must not be read as a four-pixel outline.

## Outcome

- **Code changed:** no new selector code in this ammo-refresh slice; existing
  typed Qt selector/equipment state was reviewed against the ROM and captures.
- **Behavior implemented:** selected inventory cell, one-pixel item outline,
  equipment treatment, and clean post-action redraw.
- **Internally checked:** Debug/Release field-action CTest; exact pixel
  perimeter and icon checks; whole-screen equip/unequip/child-return frames.
- **Independently visually checked:** section 06 frames `002560`, `002580`,
  `002610`, `002640`, `002699`; section 07 frames `000030`, `000120`.
- **Reported defect:** the thick selector is not reproduced in the current
  build. The current edge is one pixel at source size. The reported stale
  black-box transition is also not reproduced by current internal route
  comparisons; the exact independent weapon transition remains unavailable.
