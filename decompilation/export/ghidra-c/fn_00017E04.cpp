// Ghidra headless export
// Entry: 0x00017E04
// Name: refresh_hud_grid_if_ready
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001232 write_vdp_grid
//   0x00017C3C FUN_00017c3c


void refresh_hud_grid_if_ready(void)

{
  char in_NF;
  undefined1 in_ZF;
  char in_VF;
  
  FUN_00017c3c();
  if (!(bool)in_ZF && in_VF == in_NF) {
    write_vdp_grid();
    write_vdp_grid();
    write_vdp_grid();
  }
  return;
}


