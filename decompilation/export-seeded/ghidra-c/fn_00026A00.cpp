// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00026A00


void FUN_00026a00(void)

{
  short extraout_D1w;
  short sVar1;
  short extraout_D1w_00;
  undefined4 uVar2;
  uint unaff_D3;
  uint uVar3;
  int iVar4;
  undefined **ppuVar5;
  undefined *puVar6;
  undefined **ppuVar7;
  undefined2 uVar8;
  
  DAT_00fff0b1 = 0xff;
  FUN_000530de();
  FUN_00000f5c();
  FUN_00010952();
  FUN_00001358();
  DAT_00ffef14 = DAT_00ffef14 | 1;
  uVar2 = 0x6000;
  FUN_00026bfe();
  FUN_00026c26();
  uVar3 = unaff_D3 & 0xffff0000;
  FUN_00026bfe();
  sVar1 = 0xd;
  iVar4 = 0xe6fe;
  ppuVar5 = &PTR_DAT_0000e752;
  puVar6 = &DAT_0000dfa8;
  ppuVar7 = &PTR_DAT_0000eea8;
  do {
    FUN_00000bd2(sVar1,uVar2,uVar3,iVar4,ppuVar5,puVar6,ppuVar7);
    uVar8 = (undefined2)((uint)uVar2 >> 0x10);
    FUN_00000bf2();
    FUN_0000195c();
    iVar4 = iVar4 + 2;
    uVar3 = uVar3 & 0xffff0000;
    uVar2 = CONCAT22(uVar8,0x6000);
    FUN_00026bfe();
    ppuVar5 = (undefined **)((int)ppuVar5 + -2);
    FUN_00026bfe();
    puVar6 = puVar6 + 0x80;
    FUN_00026c26();
    ppuVar7 = ppuVar7 + -0x20;
    FUN_00026c26();
    sVar1 = extraout_D1w + -1;
  } while (sVar1 != -1);
  sVar1 = 5;
  do {
    FUN_00000bd2(sVar1,uVar2,uVar3,iVar4,ppuVar5);
    uVar8 = (undefined2)((uint)uVar2 >> 0x10);
    FUN_00000bf2();
    FUN_0000195c();
    iVar4 = iVar4 + 2;
    uVar3 = uVar3 & 0xffff0000;
    uVar2 = CONCAT22(uVar8,0x6000);
    FUN_00026bfe();
    ppuVar5 = (undefined **)((int)ppuVar5 + -2);
    FUN_00026bfe();
    sVar1 = extraout_D1w_00 + -1;
  } while (sVar1 != -1);
  FUN_00026bfe();
  sVar1 = 10;
  do {
    FUN_00000bd2();
    FUN_00000bf2();
    FUN_0000195c();
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  FUN_00026bfe();
  sVar1 = 5;
  do {
    FUN_00000bd2();
    FUN_00000bf2();
    FUN_0000195c();
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  FUN_00026bfe();
  sVar1 = 5;
  do {
    FUN_00000bd2();
    FUN_00000bf2();
    FUN_0000195c();
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  FUN_00026bfe();
  sVar1 = 10;
  do {
    FUN_00000bd2();
    FUN_00000bf2();
    FUN_0000195c();
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  return;
}


