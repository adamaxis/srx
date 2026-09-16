// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00013726


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00013726(void)

{
  undefined2 uVar1;
  byte bVar2;
  undefined2 extraout_D0u;
  undefined4 uVar3;
  short extraout_D1w;
  undefined4 extraout_D1;
  uint uVar4;
  int iVar5;
  uint uVar6;
  uint uVar7;
  undefined4 unaff_D7;
  short sVar8;
  ushort uVar9;
  ushort uVar10;
  ushort *extraout_A0;
  undefined4 extraout_A0_00;
  undefined2 *extraout_A1;
  undefined2 *extraout_A1_00;
  undefined2 *extraout_A1_01;
  undefined2 *extraout_A1_02;
  undefined2 *puVar11;
  undefined1 *puVar12;
  byte *pbVar13;
  undefined4 unaff_A4;
  undefined2 unaff_A5w;
  undefined2 unaff_A6w;
  undefined4 uVar14;
  undefined4 uVar15;
  undefined4 uVar16;
  undefined2 *puVar17;
  
  puVar11 = (undefined2 *)&DAT_00c00000;
  puVar12 = &DAT_000d4536;
  uVar7 = 4;
  do {
    FUN_0000c3e6();
    uVar6 = (uint)*extraout_A0;
    pbVar13 = puVar12 + 1;
    puVar12 = puVar12 + 2;
    bVar2 = *pbVar13;
    uVar4 = (uint)bVar2;
    FUN_00000e9c();
    *puVar11 = *extraout_A1;
    uVar1 = extraout_A1[1];
    sVar8 = extraout_D1w;
    do {
      *puVar11 = uVar1;
      sVar8 = sVar8 + -1;
    } while (sVar8 != -1);
    *puVar11 = extraout_A1[2];
    _DAT_00c00004 = 0x8f80;
    FUN_00000e9c();
    uVar1 = *extraout_A1_00;
    uVar10 = (ushort)bVar2;
    uVar9 = uVar10;
    do {
      *puVar11 = uVar1;
      uVar9 = uVar9 - 1;
    } while (uVar9 != 0xffff);
    FUN_00000e9c();
    uVar1 = *extraout_A1_01;
    do {
      *puVar11 = uVar1;
      uVar10 = uVar10 - 1;
    } while (uVar10 != 0xffff);
    _DAT_00c00004 = 0x8f02;
    iVar5 = (uVar4 + 2) * 0x80;
    FUN_00000e9c();
    *puVar11 = *extraout_A1_02;
    uVar1 = extraout_A1_02[1];
    unaff_D7 = CONCAT22((short)((uint)unaff_D7 >> 0x10),(short)extraout_D1);
    do {
      *puVar11 = uVar1;
      sVar8 = (short)unaff_D7 + -1;
      unaff_D7 = CONCAT22((short)((uint)unaff_D7 >> 0x10),sVar8);
    } while (sVar8 != -1);
    puVar17 = extraout_A1_02 + 3;
    uVar3 = CONCAT22(extraout_D0u,extraout_A1_02[2]);
    *puVar11 = extraout_A1_02[2];
    uVar15 = 0xc208;
    sVar8 = 1;
    uVar14 = extraout_D1;
    uVar16 = extraout_A0_00;
    do {
      FUN_00000bd2(uVar3,uVar14,uVar4,iVar5,uVar15,uVar6,uVar7,unaff_D7,uVar16,puVar17,puVar11,
                   puVar12,unaff_A4,unaff_A5w,unaff_A6w);
      FUN_00000bf2();
      if (DAT_00fff09e == '\0') {
        FUN_0000e6f4();
      }
      sVar8 = sVar8 + -1;
    } while (sVar8 != -1);
    uVar9 = (short)uVar7 - 1;
    uVar7 = (uint)uVar9;
  } while (uVar9 != 0xffff);
                    /* WARNING: Subroutine does not return */
  FUN_0000c0da();
}


