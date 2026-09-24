// Ghidra headless export
// Entry: 0x000189D8
// Name: write_two_hud_grids
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001232 write_vdp_grid


void write_two_hud_grids(void)

{
  write_vdp_grid();
  write_vdp_grid();
  return;
}


