// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0005AB96


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

ushort FUN_0005ab96(void)

{
  ushort uVar1;
  
  uVar1 = _DAT_00ffdf88 - _DAT_00fff11e;
  if ((0xb3 < uVar1) && (uVar1 = uVar1 & 0xb, uVar1 == 0)) {
    FUN_0005b2e2();
    FUN_0005b2e2();
    FUN_0005b2e2();
    uVar1 = FUN_0005b2e2();
  }
  return uVar1;
}


