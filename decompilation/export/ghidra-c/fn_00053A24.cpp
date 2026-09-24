// Ghidra headless export
// Entry: 0x00053A24
// Name: FUN_00053a24
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001378 FUN_00001378
//   0x00053248 FUN_00053248


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00053a24(void)

{
  int unaff_A3;
  
  FUN_00053248();
  FUN_00001378();
  *(undefined1 *)(unaff_A3 + -5) = 0;
  _DAT_00ffd980 = 1;
  DAT_00ffef14 = DAT_00ffef14 | 2;
  return;
}


