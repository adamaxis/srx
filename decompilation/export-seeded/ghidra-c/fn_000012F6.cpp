// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x000012F6


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


