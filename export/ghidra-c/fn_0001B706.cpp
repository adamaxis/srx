// Ghidra headless export
// Entry: 0x0001B706
// Name: prepare_menu_offset_once
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00017E78 calculate_menu_row_offset
//   0x0001A908 initialize_world_effect_mode


void prepare_menu_offset_once(void)

{
  if (DAT_00ffdf84 == '\0') {
    DAT_00ffdf84 = -1;
    DAT_00ffe074 = calculate_menu_row_offset();
    initialize_world_effect_mode();
  }
  return;
}

