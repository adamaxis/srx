// Ghidra headless export
// Entry: 0x00018776
// Name: FUN_00018776
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000BD2 FUN_00000bd2
//   0x00000BF2 FUN_00000bf2
//   0x00001088 FUN_00001088


void FUN_00018776(void)

{
  short sVar1;
  short sVar2;
  
  sVar2 = 4;
  do {
    sVar1 = 1;
    do {
      FUN_00000bd2();
      FUN_00000bf2();
      sVar1 = sVar1 + -1;
    } while (sVar1 != -1);
    FUN_00001088();
    DAT_00ffef14 = DAT_00ffef14 | 1;
    sVar2 = sVar2 + -1;
  } while (sVar2 != -1);
  return;
}


