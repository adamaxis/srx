// Ghidra headless export
// Entry: 0x00011928
// Name: render_menu_row
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001232 write_vdp_grid
//   0x00011AE4 adjust_menu_dialog_posture


void render_menu_row(void)

{
  write_vdp_grid();
  adjust_menu_dialog_posture();
  return;
}


