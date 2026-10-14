// Ghidra headless export
// Entry: 0x00019788
// Name: advance_matrix_program_loading
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001232 write_vdp_grid
//   0x0001792E render_menu_hud_rows
//   0x00017C52 render_menu_text_followup
//   0x00017C84 set_hud_selection_value
//   0x00017EEE advance_menu_overlay_timer


void advance_matrix_program_loading(void)

{
  short sVar1;
  char *pcVar2;
  
  DAT_00fff0ac = DAT_00fff0ac - 1;
  if ((short)((ushort)DAT_00fff0ac << 8) < 0) {
    DAT_00fff0ac = 0x28;
    render_menu_hud_rows();
  }
  advance_menu_overlay_timer();
  if (DAT_00fff0b8 != '\0') {
    pcVar2 = &DAT_00fff0ed;
    DAT_00ffdd48 = DAT_00ffdd48 + -1;
    if (DAT_00ffdd48 < 0) {
      DAT_00fff108 = 0;
      sVar1 = 4;
      do {
        if (*pcVar2 != '\x06') {
          *pcVar2 = *pcVar2 + '\x01';
          write_vdp_grid();
          DAT_00ffdd48 = *(undefined2 *)
                          ((int)&DAT_00fff0f2 + (int)(short)((ushort)DAT_00fff108 << 1));
          if (*pcVar2 != '\x06') {
            return;
          }
          set_hud_selection_value();
          render_menu_text_followup();
          return;
        }
        pcVar2 = pcVar2 + 1;
        DAT_00fff108 = DAT_00fff108 + 1;
        sVar1 = sVar1 + -1;
      } while (sVar1 != -1);
    }
  }
  return;
}


