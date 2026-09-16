// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0000AEBA


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0000aeba(void)

{
  uint uVar1;
  uint uVar2;
  short sVar3;
  undefined4 uVar4;
  short sVar5;
  
  _DAT_00ffdfec = _DAT_00ffdfec + 8;
  if (_DAT_00ffdfec < 0xa0) {
    uVar1 = (uint)_DAT_00ffdfee;
    uVar2 = (uint)_DAT_00ffdff0;
    uVar4 = *(undefined4 *)(&DAT_0000af3e + (short)((_DAT_00ffdf88 >> 2 & 3) << 2));
    sVar3 = 0x1f;
    sVar5 = 0;
    do {
      FUN_00001378(uVar1,uVar2,sVar5,uVar4);
      sVar5 = sVar5 + 8;
      sVar3 = sVar3 + -1;
    } while (sVar3 != -1);
    return;
  }
  _DAT_00ffdfea = 0;
  return;
}


