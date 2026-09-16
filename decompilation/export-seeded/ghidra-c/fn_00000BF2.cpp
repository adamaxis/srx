// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00000BF2


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

ushort FUN_00000bf2(void)

{
  ushort uVar1;
  
  uVar1 = (_DAT_00ffdd26 | _DAT_00ffdd22) & 0xff00;
  if ((uVar1 != 0) && (_DAT_00ffddc0 = 0xffff, (_DAT_00ffdd22 & 0x8000) != 0)) {
    _DAT_00ffddc2 = 0xffff;
  }
  return uVar1;
}


