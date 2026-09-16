// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00001056


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00001056(void)

{
  short sVar1;
  
  FUN_00000e9c();
  sVar1 = 0xf;
  do {
    _DAT_00c00000 = 0;
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  return;
}


