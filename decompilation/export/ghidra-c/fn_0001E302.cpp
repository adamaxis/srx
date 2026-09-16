// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0001E302


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0001e302(void)

{
  ushort uVar1;
  ushort uVar2;
  ushort extraout_D1w;
  short sVar3;
  short sVar4;
  short *psVar5;
  short *psVar6;
  bool bVar7;
  
  psVar5 = (short *)&DAT_00ffdd50;
  sVar3 = 0xbe0;
  sVar4 = 0x8c0;
  if ((ushort)(_DAT_00ffddce >> 1) < (ushort)(_DAT_00ffddc6 + 0xa00U)) {
    sVar3 = -0xbe0;
  }
  uVar1 = _DAT_00ffddc8 + 0x700;
  if (_DAT_00ffddd0 >> 1 < uVar1) {
    sVar4 = -0x8c0;
  }
  sVar3 = (sVar3 + _DAT_00ffddc6 + 0xa00U & 0xfe00) + 0x100;
  uVar2 = uVar1 & 0xfe00;
  bVar7 = (ushort)(uVar2 + 0x100) == 0;
  FUN_0001e41e();
  if (bVar7) {
    psVar5 = (short *)&DAT_00ffdd54;
    _DAT_00ffdd50 = sVar3;
    _DAT_00ffdd52 = uVar2 + 0x100;
  }
  bVar7 = (ushort)(uVar2 + 0x300) == 0;
  FUN_0001e41e();
  if (bVar7) {
    psVar6 = psVar5 + 1;
    *psVar5 = sVar3;
    psVar5 = psVar5 + 2;
    *psVar6 = uVar2 + 0x300;
  }
  bVar7 = (ushort)(uVar2 + 0x500) == 0;
  FUN_0001e41e();
  if (bVar7) {
    psVar6 = psVar5 + 1;
    *psVar5 = sVar3;
    psVar5 = psVar5 + 2;
    *psVar6 = uVar2 + 0x500;
  }
  bVar7 = (ushort)(uVar2 - 0x100) == 0;
  FUN_0001e41e();
  if (bVar7) {
    psVar6 = psVar5 + 1;
    *psVar5 = sVar3;
    psVar5 = psVar5 + 2;
    *psVar6 = uVar2 - 0x100;
  }
  bVar7 = (ushort)(uVar2 - 0x300) == 0;
  FUN_0001e41e();
  if (bVar7) {
    psVar6 = psVar5 + 1;
    *psVar5 = sVar3;
    psVar5 = psVar5 + 2;
    *psVar6 = uVar2 - 0x300;
  }
  uVar2 = extraout_D1w & 0xfe00;
  sVar3 = (sVar4 + uVar1 & 0xfe00) + 0x100;
  bVar7 = sVar3 == 0;
  FUN_0001e41e();
  if (bVar7) {
    psVar6 = psVar5 + 1;
    *psVar5 = uVar2 + 0x100;
    psVar5 = psVar5 + 2;
    *psVar6 = sVar3;
  }
  bVar7 = (ushort)(uVar2 + 0x300) == 0;
  FUN_0001e41e();
  if (bVar7) {
    psVar6 = psVar5 + 1;
    *psVar5 = uVar2 + 0x300;
    psVar5 = psVar5 + 2;
    *psVar6 = sVar3;
  }
  bVar7 = (ushort)(uVar2 + 0x500) == 0;
  FUN_0001e41e();
  if (bVar7) {
    psVar6 = psVar5 + 1;
    *psVar5 = uVar2 + 0x500;
    psVar5 = psVar5 + 2;
    *psVar6 = sVar3;
  }
  bVar7 = (ushort)(uVar2 - 0x100) == 0;
  FUN_0001e41e();
  if (bVar7) {
    psVar6 = psVar5 + 1;
    *psVar5 = uVar2 - 0x100;
    psVar5 = psVar5 + 2;
    *psVar6 = sVar3;
  }
  bVar7 = (ushort)(uVar2 - 0x300) == 0;
  FUN_0001e41e();
  if (bVar7) {
    psVar6 = psVar5 + 1;
    *psVar5 = uVar2 - 0x300;
    psVar5 = psVar5 + 2;
    *psVar6 = sVar3;
  }
  *psVar5 = 0;
  return;
}


