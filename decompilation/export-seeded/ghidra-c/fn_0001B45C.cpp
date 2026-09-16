// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0001B45C


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0001b45c(void)

{
  int iVar1;
  undefined2 uVar2;
  undefined2 uVar3;
  short sVar4;
  uint uVar5;
  short sVar6;
  short sVar7;
  short sVar8;
  int in_A0;
  short *extraout_A1;
  short *extraout_A1_00;
  short *psVar9;
  
  _DAT_00ffe0e8 = _DAT_00ffe0e8 + -1;
  if (_DAT_00ffe0e8 < 0) {
    _DAT_00ffe0e8 = _DAT_00ffe0e6;
    DAT_00ffe16a = DAT_00ffe16a + -1;
    if (DAT_00ffe16a == '\0') {
      *(byte *)(in_A0 + 0x12) = *(byte *)(in_A0 + 0x12) & 0xfb;
      FUN_00019f78();
      FUN_0001b5a6();
      return;
    }
  }
  sVar7 = 0xaf;
  uVar2 = FUN_00000d44();
  uVar3 = FUN_00000d44();
  uVar5 = CONCAT22(uVar2,uVar3);
  sVar6 = *(short *)(&DAT_0001b592 + (short)((short)(char)(DAT_00ffe16a + '\x01') << 1));
  psVar9 = extraout_A1;
  sVar8 = 0;
  do {
    iVar1 = (int)uVar5 >> 0x1f;
    uVar5 = uVar5 << 1 | -iVar1;
    if (-iVar1 == 0) {
      sVar4 = *psVar9 + 1;
    }
    else {
      sVar4 = *psVar9 + -1;
    }
    sVar8 = sVar8 - sVar4;
    if (sVar8 < 0) {
      if (sVar6 <= -sVar8) {
        sVar4 = sVar4 + -1;
      }
    }
    else if (sVar6 <= sVar8) {
      sVar4 = sVar4 + 1;
    }
    *psVar9 = sVar4;
    psVar9 = psVar9 + 2;
    sVar7 = sVar7 + -1;
    sVar8 = sVar4;
  } while (sVar7 != -1);
  sVar8 = 200;
  uVar2 = FUN_00000d44();
  uVar3 = FUN_00000d44();
  uVar5 = CONCAT22(uVar2,uVar3);
  sVar6 = (short)DAT_00ffe16a;
  if (2 < sVar6) {
    sVar6 = 2;
  }
  sVar6 = *(short *)(&DAT_0001b592 + (short)(sVar6 << 1));
  psVar9 = extraout_A1_00;
  sVar7 = 0;
  do {
    iVar1 = (int)uVar5 >> 0x1f;
    uVar5 = uVar5 << 1 | -iVar1;
    if (-iVar1 == 0) {
      sVar4 = *psVar9 + 1;
    }
    else {
      sVar4 = *psVar9 + -1;
    }
    sVar7 = sVar7 - sVar4;
    if (sVar7 < 0) {
      if (sVar6 <= -sVar7) {
        sVar4 = sVar4 + -1;
      }
    }
    else if (sVar6 <= sVar7) {
      sVar4 = sVar4 + 1;
    }
    *psVar9 = sVar4;
    psVar9 = psVar9 + 2;
    sVar8 = sVar8 + -1;
    sVar7 = sVar4;
  } while (sVar8 != -1);
  FUN_00000e18();
  DAT_00ffef14 = DAT_00ffef14 | 0xc;
  return;
}


