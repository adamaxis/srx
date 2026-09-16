// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00018776


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


