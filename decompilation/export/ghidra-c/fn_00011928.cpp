// Ghidra headless export
// Entry: 0x00011928
// Name: render_menu_row
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001232 write_vdp_grid
//   0x00011AE4 convert_menu_routine_0x00011ae4


void render_menu_row(void)

{
  write_vdp_grid();
  convert_menu_routine_0x00011ae4();
  return;
}


