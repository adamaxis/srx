// Ghidra headless export
// Entry: 0x000012F6
// Name: FUN_000012f6
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x000010D6 FUN_000010d6
//   0x00001342 FUN_00001342


void FUN_000012f6(void)

{
  short sVar1;
  
  sVar1 = 0x10;
  do {
    FUN_00001342();
    FUN_000010d6();
    DAT_00ffef14 = DAT_00ffef14 | 1;
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  return;
}


