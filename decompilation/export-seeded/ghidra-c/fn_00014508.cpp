// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00014508


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00014508(void)

{
  byte bVar1;
  short sVar2;
  short sVar3;
  ushort uVar4;
  short sVar5;
  short extraout_D1w;
  short extraout_D1w_00;
  ushort uVar6;
  short sVar7;
  int in_A0;
  int extraout_A0;
  int extraout_A0_00;
  ushort *extraout_A0_01;
  ushort *extraout_A0_02;
  ushort *extraout_A0_03;
  ushort *extraout_A0_04;
  ushort *puVar8;
  int extraout_A0_05;
  ushort *extraout_A0_06;
  int extraout_A0_07;
  ushort *extraout_A0_08;
  int extraout_A0_09;
  int extraout_A0_10;
  int extraout_A0_11;
  int iVar9;
  short *extraout_A1;
  short *extraout_A1_00;
  short *extraout_A1_01;
  short *psVar10;
  ushort *puVar11;
  bool bVar12;
  undefined1 uVar13;
  ushort local_14;
  short sStack_12;
  short sStack_10;
  short sStack_e;
  short sStack_c;
  undefined1 auStack_a [2];
  short sStack_8;
  undefined4 uStack_4;
  
  sVar7 = *(short *)(in_A0 + 0x3e);
  if ((sVar7 == 0) || ((sVar7 == 1 && (*(short *)(_DAT_00fffc04 + 0x3e) == 0)))) {
    sVar5 = *(short *)(_DAT_00fffc04 + 0x2e) + -0x6000;
  }
  else {
    sVar5 = *(short *)(_DAT_00fffc04 + 0x2e) + 0x6000;
  }
  uStack_4 = 0x14540;
  FUN_00015dcc();
  sStack_8 = *extraout_A1 + 0xe0;
  uStack_4 = CONCAT22(sVar5 + extraout_A1[2],uStack_4._2_2_);
  local_14 = 0xe0;
  sStack_10 = sVar5;
  sStack_c = sVar7;
  sVar3 = FUN_0000e7c8();
  sVar2 = sStack_10;
  puVar11 = (ushort *)auStack_a;
  iVar9 = extraout_A0;
  uVar6 = local_14;
  sVar7 = sStack_e;
  sVar5 = sStack_c;
  if ((sVar3 != 0) && (-1 < sVar3)) {
    uVar6 = -local_14;
    sStack_e = extraout_A1_00[2] - sStack_12;
    sStack_12 = uVar6 + *extraout_A1_00;
    sVar3 = FUN_0000e7c8();
    sVar7 = 1;
    puVar11 = &local_14;
    iVar9 = extraout_A0_00;
    sVar5 = sVar2;
    if ((sVar3 != 0) && (-1 < sVar3)) {
      sVar7 = *extraout_A1_01;
      sVar5 = extraout_A1_01[2];
    }
  }
  *(short *)(iVar9 + 0x96) = sVar7;
  *(short *)(iVar9 + 0x98) = sVar5;
  bVar12 = false;
  puVar11[-0xffffffff00000002] = 1;
  puVar11[-0xffffffff00000001] = 0x459c;
  FUN_000144e8();
  if (bVar12) {
    return;
  }
  uVar13 = extraout_A0_01[0xb] < 0x3c;
  if (0x3c < (short)extraout_A0_01[0xb]) {
    return;
  }
  puVar8 = extraout_A0_01;
  if ((extraout_A0_01[0x58] & 0x200) == 0) {
    bVar1 = *(byte *)(extraout_A0_01 + 0x58);
    *(byte *)(extraout_A0_01 + 0x58) = bVar1 & 0xfb;
    if ((bVar1 & 4) == 0) {
      uVar13 = extraout_A0_01[0x5c] < 0x1000;
      if (extraout_A0_01[0x5c] != 0x1000) {
        puVar11[-0xffffffff00000002] = 1;
        puVar11[-0xffffffff00000001] = 0x45da;
        FUN_00014ba6();
        puVar8 = extraout_A0_03;
        if ((extraout_A0_03[0x58] & 0x200) != 0) goto FUN_000149c0;
        puVar11[-0xffffffff00000002] = 1;
        puVar11[-0xffffffff00000001] = 0x45e8;
        FUN_00014c2e();
        puVar8 = extraout_A0_04;
      }
    }
    else {
      puVar11[-0xffffffff00000002] = 1;
      puVar11[-0xffffffff00000001] = 0x45c8;
      FUN_00014c2e();
      puVar8 = extraout_A0_02;
    }
LAB_000145e8:
    uVar4 = puVar8[0x4b];
    uVar6 = puVar8[0x4c];
    puVar11[-0xffffffff00000002] = 1;
    puVar11[-0xffffffff00000001] = 0x45fc;
    FUN_00015d3c();
    iVar9 = extraout_A0_05;
  }
  else {
FUN_000149c0:
    if ((puVar8[0x58] & 0x800) != 0) {
      puVar11[-0xffffffff00000002] = 1;
      puVar11[-0xffffffff00000001] = 0x49ce;
      FUN_00014914();
      puVar8 = extraout_A0_06;
      if ((bool)uVar13) {
        *(byte *)(extraout_A0_06 + 0x58) = *(byte *)(extraout_A0_06 + 0x58) & 0xfd;
        *(byte *)(extraout_A0_06 + 0x58) = *(byte *)(extraout_A0_06 + 0x58) & 0xf7;
        return;
      }
    }
    sVar7 = (puVar8[2] >> 4 & 0x7fe0) * 2 + (*puVar8 >> 9);
    psVar10 = (short *)((int)puVar8 + (short)puVar8[0x5d] + 0xbc);
    if (sVar7 == *psVar10) {
      if (psVar10[1] == -1) {
        if ((puVar8[0x58] & 0x800) != 0) {
          puVar11[-0xffffffff00000002] = 1;
          puVar11[-0xffffffff00000001] = 0x4a64;
          FUN_00014902();
          uVar6 = *(ushort *)(extraout_A0_09 + 0x34) >> 0xd & 6;
          if (((&DAT_00ff1700)[(short)(*(short *)(&DAT_000148ca + (short)uVar6) + sVar7)] == '\0')
             && ((&DAT_00ff1700)
                 [(short)(*(short *)(&DAT_000148ca + (short)(uVar6 + 2 & 6)) +
                         *(short *)(&DAT_000148ca + (short)(uVar6 + 4 & 6)) + sVar7)] == '\0')) {
            puVar11[-0xffffffff00000002] = 1;
            puVar11[-0xffffffff00000001] = 0x4ab0;
            FUN_000148aa();
            sVar5 = extraout_D1w;
            iVar9 = extraout_A0_10;
          }
          else {
            sVar5 = 4;
            while( true ) {
              puVar11[-1] = sVar5;
              puVar11[-0xffffffff00000003] = 1;
              puVar11[-0xffffffff00000002] = 0x4ac6;
              FUN_00014764();
              sVar5 = extraout_D1w_00;
              iVar9 = extraout_A0_11;
              if (extraout_D1w_00 != sVar7) break;
              sVar5 = puVar11[-1] + -1;
              if (sVar5 == -1) {
                FUN_00014756();
                return;
              }
            }
          }
          *(undefined2 *)(iVar9 + 0xba) = 0;
          *(short *)(iVar9 + 0xbc) = sVar5;
          *(undefined2 *)(iVar9 + 0xbe) = 0xffff;
          return;
        }
        *(byte *)(puVar8 + 0x58) = *(byte *)(puVar8 + 0x58) & 0xfd;
        *(byte *)(puVar8 + 0x58) = *(byte *)(puVar8 + 0x58) & 0xf7;
        puVar11[-0xffffffff00000002] = 1;
        puVar11[-0xffffffff00000001] = 0x4a50;
        FUN_00014c2e();
        puVar8 = extraout_A0_08;
        goto LAB_000145e8;
      }
      puVar8[0x5d] = puVar8[0x5d] + 2;
    }
    puVar11[-0xffffffff00000002] = 1;
    puVar11[-0xffffffff00000001] = 0x4a12;
    FUN_00014b4e();
    puVar11[-1] = uVar6 << 0xd | uVar6 >> 3;
    puVar11[-0xffffffff00000003] = 1;
    puVar11[-0xffffffff00000002] = 0x4a2e;
    uVar4 = FUN_00015ca4();
    uVar6 = puVar11[-1];
    iVar9 = extraout_A0_07;
  }
  *(ushort *)(iVar9 + 0x34) = uVar6;
  *(ushort *)(iVar9 + 200) = uVar4;
  *(undefined1 **)(iVar9 + 0x3a) = &LAB_0000b574;
  return;
}


