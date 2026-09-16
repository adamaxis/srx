// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0005B08E


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

ushort FUN_0005b08e(void)

{
  ushort uVar1;
  ushort uVar2;
  
  uVar1 = _DAT_00ffd8a0;
  uVar2 = _DAT_00ffdf88 & 7;
  if ((_DAT_00ffdf88 & 7) == 0) {
    _DAT_00ffd8a0 = _DAT_00ffd8a2;
    _DAT_00ffd8a2 = _DAT_00ffd8a4;
    _DAT_00ffd8a4 = uVar1;
    DAT_00ffef14 = DAT_00ffef14 | 1;
    uVar2 = uVar1;
  }
  return uVar2;
}


