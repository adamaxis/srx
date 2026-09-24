// Ghidra headless export
// Entry: 0x0001D2E6
// Name: FUN_0001d2e6
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0001D33A FUN_0001d33a


void FUN_0001d2e6(void)

{
  short *psVar1;
  
  for (psVar1 = DAT_00ffdf7a; *psVar1 != -0x8000; psVar1 = psVar1 + 4) {
    FUN_0001d33a();
  }
  DAT_00ffdf82 = DAT_00ffdf82 + -1;
  if (DAT_00ffdf82 < 0) {
    DAT_00ffdf82 = 0;
    DAT_00ffdf70 = DAT_00ffdf70 + 1;
  }
  if (DAT_00ffdf70 <= DAT_00ffdf78) {
    return;
  }
  return;
}


