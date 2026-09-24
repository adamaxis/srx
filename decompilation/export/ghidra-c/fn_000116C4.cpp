// Ghidra headless export
// Entry: 0x000116C4
// Name: write_menu_grid_quad
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001232 write_vdp_grid
//   0x00012C5A write_menu_repeated_grid_with_metadata


void write_menu_grid_quad(void)

{
  write_vdp_grid();
  write_vdp_grid();
  write_vdp_grid();
  write_vdp_grid();
  write_menu_repeated_grid_with_metadata();
  return;
}


