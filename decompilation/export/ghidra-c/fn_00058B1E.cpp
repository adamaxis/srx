// Ghidra headless export
// Entry: 0x00058B1E
// Name: convert_intro_routine_0x00058b1e
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001232 write_vdp_grid


void convert_intro_routine_0x00058b1e(void)

{
  short sVar1;
  
  sVar1 = 5;
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


