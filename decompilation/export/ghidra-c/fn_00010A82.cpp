// Ghidra headless export
// Entry: 0x00010A82
// Name: convert_menu_routine_0x00010a82
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001232 write_vdp_grid


void convert_menu_routine_0x00010a82(void)

{
  VDP_CTRL = 0x8134;
  write_vdp_grid();
  write_vdp_grid();
  write_vdp_grid();
  write_vdp_grid();
  VDP_CTRL = 0x8174;
  return;
}


