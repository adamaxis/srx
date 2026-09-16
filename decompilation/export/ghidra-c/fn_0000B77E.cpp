// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0000B77E


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

short FUN_0000b77e(void)

{
  undefined2 in_D0w;
  short sVar1;
  int extraout_A0;
  
  _DAT_00ffdf66 = in_D0w;
  sVar1 = FUN_0000b48c();
  if ((sVar1 < 0) && ((*(byte *)(extraout_A0 + 0x10) & 2) == 0)) {
    sVar1 = 0x7f;
  }
  return sVar1;
}


