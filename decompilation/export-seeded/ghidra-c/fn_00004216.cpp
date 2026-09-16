// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00004216


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

short FUN_00004216(void)

{
  short sVar1;
  
  sVar1 = (_DAT_00ffddc6 >> 4) - 8;
  if ((sVar1 < 0) || (sVar1 = (_DAT_00ffddc6 >> 4) - 0x10, sVar1 < 0)) {
    sVar1 = sVar1 + 8;
  }
  return sVar1;
}


