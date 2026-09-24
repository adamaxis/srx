// Ghidra headless export
// Entry: 0x000194AC
// Name: advance_hud_selection_row
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001232 write_vdp_grid
//   0x0001792E render_menu_hud_rows
//   0x00019504 write_hud_panel_grid


void advance_hud_selection_row(void)

{
  char unaff_D5b;
  
  write_hud_panel_grid();
  DAT_00fff0d4 = unaff_D5b + DAT_00fff0d4;
  if ((short)((ushort)DAT_00fff0d4 << 8) < 0) {
    DAT_00fff0d4 = 4;
  }
  else if (-1 < (char)(DAT_00fff0d4 - 5)) {
    DAT_00fff0d4 = 0;
  }
  write_vdp_grid();
  render_menu_hud_rows();
  return;
}


