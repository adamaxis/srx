// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0000B732


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

ushort FUN_0000b732(void)

{
  ushort uVar1;
  ushort uVar2;
  
  uVar2 = FUN_0000b77e();
  if (((short)uVar2 < 0) &&
     (((_DAT_00ffdf84 != 0 || (uVar1 = uVar2 & 0x1f, 0x17 < uVar1)) ||
      (((_DAT_00ffddc4 == 0x14 || (_DAT_00ffdf40 == 2)) && ((1 < uVar1 && (uVar1 < 6)))))))) {
    return 0x7f;
  }
  return uVar2;
}


