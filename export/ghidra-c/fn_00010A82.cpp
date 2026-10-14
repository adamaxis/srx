// Ghidra headless export
// Entry: 0x00010A82
// Name: refresh_menu_grid_quadruple
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001232 write_vdp_grid


void refresh_menu_grid_quadruple(void)

{
  VDP_CTRL = 0x8134;
  write_vdp_grid();
  write_vdp_grid();
  write_vdp_grid();
  write_vdp_grid();
  VDP_CTRL = 0x8174;
  return;
}


