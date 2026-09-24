// Ghidra headless export
// Entry: 0x00006148
// Name: FUN_00006148
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000F5C FUN_00000f5c


void FUN_00006148(void)

{
  short sVar1;
  
  sVar1 = DAT_00ffdf2a + -1;
  do {
    FUN_00000f5c();
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  DAT_00ffdf2a = 0;
  return;
}


