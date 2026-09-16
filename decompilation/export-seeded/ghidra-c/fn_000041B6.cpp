// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x000041B6


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

ushort FUN_000041b6(void)

{
  ushort uVar1;
  
  uVar1 = _DAT_00ffddc6 >> 4;
  if (-1 < (short)(uVar1 - 8)) {
    uVar1 = uVar1 - 8;
  }
  return uVar1;
}


