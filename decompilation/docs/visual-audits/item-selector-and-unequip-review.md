# Inventory Selector and Equip/Unequip Review

Source/code audit: 2026-10-07. Full-screen re-review: 2026-10-08 and 2026-10-09.
Primary source: US ROM `decompilation/input/shadowrun_genesis_us.bin`.
Qt route: `shadowrun_ghidra_conversion_qt_field_action_qa`.

## Source and typed state

- ROM `0x11506` changes the selected inventory coordinates; `0x11592` builds
  the cursor sprite from those coordinates. Selection is independent of
  possession: after discarding the selected item, the cursor remains on the
  now-empty cell. There is no item-presence gate in the traced cursor path.
- The item cursor is a one-pixel `#C4BE27` outline on the 24x16 item image.
  The equipped-cell treatment is separate and comes through `0x11624` / `0x11678`.
  In the Qt scene, that cell treatment is drawn below the direct item artwork;
  the cursor is then drawn on the selected item coordinates.
- ROM `0x1219C` checks whether any armor is equipped. If so, it clears the
  runner's `+0x76` armor index and the selected cell's `+0x6E` phase byte,
  even when the selected item is a different armor. A later action takes the
  `0x121B0` equip branch and writes the new armor index and selected phase.
  Weapon/gear state at `+0x56/+0x57/+0x4E` remains separate.
- The Qt runtime binds these transitions to the active typed runner record and
  rebuilds the menu frame after the action returns through `0x11928`.
- `items.lua` now returns explicit ordered text keys for the 14 inventory and
  First Aid messages and the 27 weapon power/armor detail labels. Script QA
  checks each stable ID/key pair; field-menu catalog QA checks that the keys
  resolve to the imported ROM copy. This content binding did not change native
  menu state or screen layout.

## Nine-question visual review

1. **Does it look correct? Yes for the selector geometry.** The selected item
   perimeter measures exactly 76 `#C4BE27` pixels, the perimeter count for a
   24x16 one-pixel rectangle. There are no matching-color pixels inside the
   edge. Independent section 06 frames `002560`, `002580`, `002610`, and
   `002640` show the same slim selection marker. The selected empty-cell case
   is source-derived and internally captured; no matching independent empty
   cell frame was found. The exact armor switch transition is also absent from
   the recording.
2. **Does the look match behavioral state? Yes.** Primary and secondary
   coordinates select the typed slot, including an empty slot after discard.
   The equipped marker follows weapon-slot state, armor `+0x76`, or the typed
   gear slot; it is not inferred from item possession alone. Debug/Release QA
   checks armor codes `0x2C` and `0x2D`, gear `0x15`, their selected cells,
   phase/state, the source's two-step armor change, and stable Lua text keys
   for inventory/First Aid copy and item display labels.
3. **Are the visuals themselves correct? Yes for the reviewed routes.** Direct
   named weapon and armor icon assets match the visible icon interior. The
   one-pixel cursor and separate equipped-cell treatment are independently
   asserted. Weapon 1, weapon 2, armor, and gear `0x15` grids after unequip
   match fresh renders pixel-for-pixel. Gear's direct miscellaneous icon uses
   the asset variant selected by typed equipment state; equip and unequip
   restore the corresponding icon and equipment-cell treatment. Repeated
   equipment changes leave no black cell box or stale control. The previously
   reported black artifact is not reproduced by the current routes. The
   source-derived two-step armor replacement behavior was corrected in the
   typed mutation.
4. **Should anything animate? No.** The cursor and item-action redraw have no
   animation counter in the traced path. Debug/Release QA compares full
   equipped and unequipped armor and gear frames after 30 idle updates; they
   remain pixel-identical.
5. **Does it have sound?** Inventory cursor movement at `0x11506` has no
   source sound call; QA verifies that vertical and horizontal cursor movement
   add no event. The source action-row confirm requests event `0x24`; moving
   within the action row uses `0x25`. Weapon, armor, and gear action routes
   check the expected `0x24` requests. Shared audio QA decodes and starts the
   mapped resource and checks its duration (2.484 seconds including its
   release tail). Windows loopback for this exact inventory visit remains
   unrecorded.
6. **Are the other visible parts of the screen also correct? Yes for the
   reviewed route.** All 54 Debug outputs are full 320x224 frames and were
   inspected. The runner portrait, right HUD, item detail, posture strip,
   action strip, Clips child, confirmation rows, and return screen retain their
   expected state. The gear frames show its icon variant and equipment marker
   changing with typed state, with the rest of the panel intact. Child controls
   disappear on return; no old button or panel is left beneath the next screen.
7. **Do multiple frames support the interpretation? Yes, with limits.** The
   independent section 06 sequence `002550`, `002560`, `002580`, `002610`,
   `002640`, and `002699` supports the shared inventory shell, selected
   armor/item treatment, First Aid child, and return. Section 07 frames
   `000030` and `000120` show the following result and field state. These do
   not show the precise weapon, armor, or gear equip/unequip or replacement
   input sequence.
   Internal QA covers open, cancel, equip, idle, unequip, repeat, replacement,
   weapon switch, reload, discard, Clips return, gear equip/unequip, and parent
   redraw.
8. **Does the new UI replace the old UI cleanly? Yes.** The item action row
   replaces the lower inventory controls. Clips replaces the center panel and
   its Cancel path restores the parent command page pixel-identically. Armor
   action return is compared with a fresh full-screen parent render and with
   the exact pre-equip frame.
9. **Was the complete affected interval reviewed? Yes for the item-action
   defect route.** The dump starts from a selected armor cell, includes the
   action row, equip, a 30-frame hold, unequip, a second hold, repeated
   toggles, selection of a second armor, source two-step replacement, gear
   equip/unequip, and the final parent state. The test process exits normally.
   The full route does not claim to cover the earlier field-to-command-menu
   opening animation.

## Full-screen files reviewed

Debug output directory:
`build/conversion/qt/debug/qt_runtime/field-action-qa/`. Release output is
under `build/conversion/qt/release/qt_runtime/field-action-qa/`.

- Inventory action and redraw: `00_inventory_action_equip.bmp`,
  `01_inventory_action_cancel.bmp`, `02_inventory_action_reselected.bmp`,
  `03_inventory_action_unselected.bmp`, `04_inventory_action_reselected.bmp`,
  `05_fresh_unequipped_baseline.bmp`, `05_weapon_1_unequipped.bmp`,
  `05_weapon_1_unequipped_frame30.bmp`, `06_combat_bars_after_equip.bmp`,
  `07_discard_confirmation_no.bmp`, `08_discard_confirmation_yes.bmp`,
  `09_inventory_after_discard.bmp`.
- Weapons and Clips: `10_weapon_2_equipped_at_50_percent.bmp`,
  `10_weapon_2_equipped_at_50_percent_frame30.bmp`,
  `11_fresh_weapon_2_unequipped_baseline.bmp`,
  `11_weapon_2_unequipped_at_50_percent.bmp`,
  `11_weapon_2_unequipped_at_50_percent_frame30.bmp`,
  `12_weapon_1_reloaded_from_50_percent.bmp`,
  `13_reload_confirmation_no.bmp`, `14_reload_confirmation_yes.bmp`,
  `15_weapon_1_reloaded_to_capacity.bmp`,
  `16_clips_closed_parent_redraw.bmp`,
  `17_clips_cancel_clears_child_controls.bmp`,
  `18_weapon_1_unequip_redraw_repeat.bmp`,
  `19_weapon_2_equipped_full_capacity.bmp`,
  `20_weapon_2_empty_magazine.bmp`, `21_weapon_2_reloaded_to_capacity.bmp`,
  `actual.bmp`.
- Armor 1 equip/unequip: `22_armor_before_equip.bmp`,
  `22_armor_action_menu.bmp`, `23_armor_equipped.bmp`,
  `23_armor_equipped_frame30.bmp`,
  `23_fresh_armor_equipped_baseline.bmp`,
  `24_armor_unequip_action_menu.bmp`, `25_armor_unequipped.bmp`,
  `25_armor_unequipped_frame30.bmp`,
  `25_fresh_armor_unequipped_baseline.bmp`,
  `26_armor_unequipped_repeat.bmp`.
- Armor replacement: `27_armor_one_before_replacement.bmp`,
  `28_armor_replacement_selected.bmp`,
  `29_armor_replacement_toggled_off.bmp`,
  `29_fresh_armor_replacement_off_baseline.bmp`,
  `30_armor_replacement_equipped.bmp`,
  `30_fresh_armor_replacement_equipped_baseline.bmp`,
  `31_armor_replacement_unequipped.bmp`,
  `31_fresh_armor_replacement_unequipped_baseline.bmp`.
- Gear `0x15`: `32_gear_before_equip.bmp`, `32_gear_action_menu.bmp`,
  `33_fresh_gear_equipped_baseline.bmp`, `33_gear_equipped.bmp`,
  `33_gear_equipped_frame30.bmp`,
  `34_fresh_gear_unequipped_baseline.bmp`, `34_gear_unequipped.bmp`,
  `34_gear_unequipped_frame30.bmp`.

All 55 shared Debug/Release BMPs have matching SHA-256 hashes. The old
debug-only `item-cell-zoom.bmp` is a 4x nearest-neighbor crop; each four-pixel
display edge represents one source pixel and is not a four-pixel outline.

On 2026-10-08, I reopened all 54 Release BMPs at their full 320x224 size,
including the weapon, armor, and gear unequip frames, repeated toggles, fresh
unequipped baselines, child-page return, and `actual.bmp`. The reported black
box did not reproduce in this sequence; old controls and child overlays were
not left over on the parent panel. The matching Debug outputs were confirmed
by SHA-256 in the current QA set. This internal renderer comparison still
does not provide an independent Genesis image of the exact equip/unequip
transition, so `EQUIP-02` remains open.

On 2026-10-09, the current Debug and Release runs emitted 55 shared full-screen
frames, including the default-actor field baseline. Four contact sheets were
regenerated and reviewed across the inventory action, discard, weapon/Clips,
armor replacement, and gear sequences. The separate First Aid, automatic-use,
and Trade sets add 7, 6, and 10 matching frames; those complete compositions
were also reviewed. Native-size checkpoints for the action row, unequipped
inventory, First Aid target, automatic-use result, and Trade confirmation
were opened individually. No old button row, stale overlay, or new text overlap
appeared after the Lua key migration.

## Outcome

- **Code changed:** the typed armor toggle now follows the ROM's any-armor
  clear branch before equipping a different armor; Debug/Release Qt QA covers
  its full-screen transition and gear `0x15`'s state-dependent icon asset and
  equipment marker. The selector drawing itself did not need a new runtime
  change.
- **Behavior implemented:** selected coordinates persist independently from
  possession; equipment is typed state; armor replacement is two-step; the
  complete action strip and parent redraw are checked.
- **Internally checked:** Debug and Release builds; the field action QA and
  inventory action conversion test pass in both. All 55 field-action frames
  and the First Aid, automatic-use, and Trade route sets match across
  configurations. The test checks
  direct asset pixels, exact selector geometry, audio events, 30-frame holds,
  full-grid fresh-render matches, and complete return screens.
- **Independently visually checked:** section 06 frames `002550`, `002560`,
  `002580`, `002610`, `002640`, `002699`; section 07 frames `000030` and
  `000120`. No exact armor toggle or replacement capture is available.
- **Reported defect status:** the thick selector and stale black box are not
  reproduced in the current full-screen routes. The selector geometry is
  independently supported. The exact black-box equip/unequip transition is
  still unavailable in independent footage, so `EQUIP-02` remains open rather
  than being claimed visually resolved.
