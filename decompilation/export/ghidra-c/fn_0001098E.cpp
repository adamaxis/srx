// Ghidra headless export
// Entry: 0x0001098E
// Name: clear_magic_menu_grids
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001232 write_vdp_grid


void clear_magic_menu_grids(void)

{
  write_vdp_grid();
  write_vdp_grid();
  write_vdp_grid();
  write_vdp_grid();
  write_vdp_grid();
  write_vdp_grid();
  write_vdp_grid();
  write_vdp_grid();
  write_vdp_grid();
  if (DAT_00fff0af != '\0') {
    write_vdp_grid();
  }
  VDP_CTRL = 0x8174;
  return;
}


