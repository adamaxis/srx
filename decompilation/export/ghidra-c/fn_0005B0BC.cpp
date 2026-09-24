// Ghidra headless export
// Entry: 0x0005B0BC
// Name: FUN_0005b0bc
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0005B100 FUN_0005b100
//   0x0005B1B6 FUN_0005b1b6


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0005b0bc(void)

{
  DAT_00ffdd02 = 1;
  FUN_0005b100();
  FUN_0005b1b6();
  _DAT_00ffd980 = 0;
  DAT_00ffef14 = DAT_00ffef14 | 2;
  return;
}


