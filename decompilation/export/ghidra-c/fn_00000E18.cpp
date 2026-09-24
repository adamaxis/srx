// Ghidra headless export
// Entry: 0x00000E18
// Name: FUN_00000e18
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000E9C FUN_00000e9c


void FUN_00000e18(void)

{
  short unaff_D7w;
  word *extraout_A0;
  word *pwVar1;
  
  FUN_00000e9c();
  pwVar1 = extraout_A0;
  do {
    VDP_DATA = *pwVar1;
    unaff_D7w = unaff_D7w + -1;
    pwVar1 = pwVar1 + 1;
  } while (unaff_D7w != -1);
  return;
}


