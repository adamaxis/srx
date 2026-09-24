// Ghidra headless export
// Entry: 0x00011900
// Name: prepare_menu_row_grid
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x000011C0 write_repeated_vdp_grid
//   0x00012C56 write_menu_repeated_grid


void prepare_menu_row_grid(void)

{
  write_menu_repeated_grid();
  write_repeated_vdp_grid();
  return;
}


