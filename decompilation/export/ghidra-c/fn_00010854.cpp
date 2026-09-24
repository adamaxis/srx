// Ghidra headless export
// Entry: 0x00010854
// Name: write_three_grids_then_service
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x000011C0 write_repeated_vdp_grid
//   0x00010156 write_repeated_menu_grid


void write_three_grids_then_service(void)

{
  write_repeated_vdp_grid();
  write_repeated_vdp_grid();
  write_repeated_vdp_grid();
  write_repeated_menu_grid();
  return;
}


