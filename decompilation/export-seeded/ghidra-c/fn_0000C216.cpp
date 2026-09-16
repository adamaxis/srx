// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0000C216


/* WARNING: Removing unreachable block (ram,0x0000c25a) */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined8 FUN_0000c216(void)

{
  undefined4 in_D0;
  uint uVar1;
  short sVar2;
  undefined4 in_D1;
  uint extraout_D1;
  uint uVar3;
  ushort unaff_D2w;
  short sVar4;
  int extraout_A0;
  
  sVar4 = 4;
  do {
    FUN_00000e9c();
    sVar4 = sVar4 + -1;
    if (sVar4 == 0) {
      sVar2 = 0x18;
      sVar4 = 4;
      uVar3 = extraout_D1;
    }
    else {
      uVar1 = extraout_D1;
      if (extraout_D1 != 0) {
        uVar1 = extraout_D1 % 10 << 0x10 | extraout_D1 / 10 & 0xffff;
      }
      uVar3 = uVar1 & 0xffff;
      sVar2 = (short)(uVar1 >> 0x10) * 2 + 0x20;
    }
    _DAT_00c00000 = unaff_D2w | *(ushort *)(extraout_A0 + sVar2) | 0x8000;
  } while ((short)uVar3 != 0);
  return CONCAT44(in_D0,in_D1);
}


