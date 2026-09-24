// Ghidra headless export
// Entry: 0x0000131C
// Name: FUN_0000131c
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001088 FUN_00001088
//   0x00001342 FUN_00001342


void FUN_0000131c(void)

{
  short sVar1;
  
  sVar1 = 0x10;
  do {
    FUN_00001342();
    FUN_00001088();
    DAT_00ffef14 = DAT_00ffef14 | 1;
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  return;
}


