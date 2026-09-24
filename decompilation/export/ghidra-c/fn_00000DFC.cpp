// Ghidra headless export
// Entry: 0x00000DFC
// Name: FUN_00000dfc
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000E9C FUN_00000e9c


void FUN_00000dfc(void)

{
  word unaff_D2w;
  short unaff_D7w;
  
  FUN_00000e9c();
  do {
    VDP_DATA = unaff_D2w;
    unaff_D7w = unaff_D7w + -1;
  } while (unaff_D7w != -1);
  return;
}


