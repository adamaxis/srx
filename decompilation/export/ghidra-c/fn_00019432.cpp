// Ghidra headless export
// Entry: 0x00019432
// Name: handle_menu_directional_input
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00017BC0 refresh_menu_number_or_grid
//   0x00017C84 set_hud_selection_value
//   0x000194AC advance_hud_selection_row


void handle_menu_directional_input(void)

{
  ushort uVar1;
  
  if (((DAT_00ffdd22._0_1_ & 4) != 0) || ((DAT_00ffdd22._0_1_ & 8) != 0)) {
    uVar1 = (ushort)DAT_00fff0d4;
    if ((((&DAT_00fffbd8)[(short)uVar1] != '\x03') &&
        ((((&DAT_00fffbd8)[(short)uVar1] != '\x05' && ((&DAT_00fffbd8)[(short)uVar1] != '\x06')) &&
         ((&DAT_00fffbd8)[(short)uVar1] != '\a')))) &&
       (-1 < (char)((&DAT_00fff0e8)[(short)uVar1] + -6))) {
      set_hud_selection_value();
    }
    advance_hud_selection_row();
    refresh_menu_number_or_grid();
  }
  return;
}


