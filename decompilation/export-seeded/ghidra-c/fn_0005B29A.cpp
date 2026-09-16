// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0005B29A


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0005b29a(void)

{
  short sVar1;
  
  FUN_00000e9c();
  sVar1 = 0x3ff;
  do {
    _DAT_00c00000 = 0x7ff07ff;
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  return;
}


