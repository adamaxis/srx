// Ghidra headless export
// Entry: 0x00011ABE
// Name: prepare_menu_dialog
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00011AE4 convert_menu_routine_0x00011ae4
//   0x00012CEE update_equipment_combat_bars


undefined8 prepare_menu_dialog(void)

{
  undefined8 uVar1;
  
  uVar1 = convert_menu_routine_0x00011ae4();
  update_equipment_combat_bars();
  update_equipment_combat_bars();
  return uVar1;
}


