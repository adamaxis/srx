// Ghidra headless export
// Entry: 0x0000FDAE
// Name: write_menu_grid_quadruple
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001232 write_vdp_grid


void write_menu_grid_quadruple(void)

{
  short sVar1;
  
  sVar1 = 1;
  do {
    write_vdp_grid();
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  sVar1 = 1;
  do {
    write_vdp_grid();
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  return;
}


