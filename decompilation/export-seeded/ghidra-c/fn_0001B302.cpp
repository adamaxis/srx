// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0001B302


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0001b302(void)

{
  ushort uVar1;
  ushort uVar2;
  ushort uVar3;
  short sVar4;
  short *extraout_A0;
  
  _DAT_00ffe086 = _DAT_00ffe086 + 1;
  if (_DAT_00ffe086 < 0x3c) {
    sVar4 = 0x57;
    do {
      uVar1 = FUN_00000d44();
      uVar2 = uVar1 & 0x1f;
      if ((uVar1 & 0x80) != 0) {
        uVar2 = -uVar2;
      }
      uVar3 = uVar1 >> 8 & 0x1f;
      if ((uVar1 >> 8 & 0x80) != 0) {
        uVar3 = -uVar3;
      }
      *extraout_A0 = uVar2 + *extraout_A0;
      extraout_A0[1] = uVar2 + extraout_A0[1];
      extraout_A0[2] = uVar3 + extraout_A0[2];
      extraout_A0[3] = uVar3 + extraout_A0[3];
      sVar4 = sVar4 + -1;
    } while (sVar4 != -1);
    FUN_00000e18();
    DAT_00ffef14 = DAT_00ffef14 | 0xc;
    return;
  }
  _DAT_00ffe084 = 0;
  FUN_0001b5a6();
  return;
}


