// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0000286E


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0000286e(void)

{
  short *psVar1;
  int iVar2;
  undefined2 uVar3;
  short sVar4;
  int iVar5;
  short *extraout_A1;
  bool bVar6;
  
  iVar2 = _DAT_00fffc3c;
  if (_DAT_00fffc3c == 0) {
    return;
  }
  iVar5 = 0xff0400;
  sVar4 = 0xc;
  while (((*(byte *)(iVar5 + 0x10) & 1) == 0 || ((*(byte *)(iVar5 + 0xb0) & 0x80) == 0))) {
    iVar5 = iVar5 + 0x100;
    sVar4 = sVar4 + -1;
    if (sVar4 == -1) {
      bVar6 = false;
      uVar3 = FUN_0001a342();
      if (!bVar6) {
        bVar6 = false;
        _DAT_00ffdd40 = uVar3;
        FUN_0001a2cc();
        if (!bVar6) {
          FUN_000044bc();
          *(byte *)(extraout_A1 + 0x58) = *(byte *)(extraout_A1 + 0x58) | 0x80;
          psVar1 = _DAT_00fffc04;
          _DAT_00fffc3c = iVar2;
          *extraout_A1 = *_DAT_00fffc04 + 0x80;
          extraout_A1[2] = psVar1[2];
          extraout_A1[0x4b] = *extraout_A1;
          extraout_A1[0x4c] = extraout_A1[2];
        }
      }
      return;
    }
  }
  return;
}


