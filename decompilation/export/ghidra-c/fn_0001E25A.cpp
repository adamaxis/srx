// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0001E25A


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0001e25a(void)

{
  short sVar1;
  undefined2 uVar2;
  short sVar3;
  short *extraout_A1;
  short *unaff_A4;
  short *psVar4;
  bool bVar5;
  
  bVar5 = false;
  _DAT_00ffdd4c = unaff_A4;
  uVar2 = FUN_0001a342();
  if (!bVar5) {
    bVar5 = false;
    _DAT_00ffdd40 = uVar2;
    FUN_0001a2cc();
    if (!bVar5) {
      FUN_000044bc();
      *(undefined1 *)((int)extraout_A1 + 0x4d) = 4;
      *(byte *)(extraout_A1 + 8) = *(byte *)(extraout_A1 + 8) | 0x20;
      psVar4 = _DAT_00ffdd4c + 1;
      sVar3 = *_DAT_00ffdd4c;
      if (*_DAT_00ffdd4c == 0) {
        psVar4 = (short *)&DAT_00ffdd52;
        sVar3 = _DAT_00ffdd50;
      }
      _DAT_00ffdd4c = psVar4 + 1;
      sVar1 = *psVar4;
      *extraout_A1 = sVar3;
      extraout_A1[2] = sVar1;
      DAT_00ffe177 = DAT_00ffe177 + '\x01';
      *(byte *)(extraout_A1 + 0x4f) = *(byte *)(extraout_A1 + 0x4f) | 0x40;
      extraout_A1[0x4b] = *extraout_A1;
      extraout_A1[0x4c] = extraout_A1[2];
      FUN_0001dbaa();
      return;
    }
  }
  return;
}


