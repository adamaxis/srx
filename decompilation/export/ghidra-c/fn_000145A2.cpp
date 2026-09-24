// Ghidra headless export
// Entry: 0x000145A2
// Name: FUN_000145a2
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00014BA6 FUN_00014ba6
//   0x00014C2E FUN_00014c2e
//   0x00015D3C FUN_00015d3c


void FUN_000145a2(void)

{
  byte bVar1;
  short sVar2;
  ushort uVar3;
  short sVar4;
  short extraout_D1w;
  short extraout_D1w_00;
  short sVar5;
  ushort unaff_D2w;
  ushort uVar6;
  ushort *in_A0;
  ushort *extraout_A0;
  ushort *extraout_A0_00;
  ushort *extraout_A0_01;
  int extraout_A0_02;
  ushort *extraout_A0_03;
  int extraout_A0_04;
  ushort *extraout_A0_05;
  int extraout_A0_06;
  int extraout_A0_07;
  int extraout_A0_08;
  int iVar7;
  short *psVar8;
  undefined1 uVar9;
  
  uVar9 = in_A0[0xb] < 0x3c;
  if (0x3c < (short)in_A0[0xb]) {
    return;
  }
  if ((in_A0[0x58] & 0x200) == 0) {
    bVar1 = *(byte *)(in_A0 + 0x58);
    *(byte *)(in_A0 + 0x58) = bVar1 & 0xfb;
    if ((bVar1 & 4) == 0) {
      uVar9 = in_A0[0x5c] < 0x1000;
      if (in_A0[0x5c] != 0x1000) {
        FUN_00014ba6();
        in_A0 = extraout_A0_00;
        if ((extraout_A0_00[0x58] & 0x200) != 0) goto FUN_000149c0;
        FUN_00014c2e();
        in_A0 = extraout_A0_01;
      }
    }
    else {
      FUN_00014c2e();
      in_A0 = extraout_A0;
    }
LAB_000145e8:
    uVar3 = in_A0[0x4b];
    uVar6 = in_A0[0x4c];
    FUN_00015d3c();
    iVar7 = extraout_A0_02;
  }
  else {
FUN_000149c0:
    if (((in_A0[0x58] & 0x800) != 0) && (FUN_00014914(), in_A0 = extraout_A0_03, (bool)uVar9)) {
      *(byte *)(extraout_A0_03 + 0x58) = *(byte *)(extraout_A0_03 + 0x58) & 0xfd;
      *(byte *)(extraout_A0_03 + 0x58) = *(byte *)(extraout_A0_03 + 0x58) & 0xf7;
      return;
    }
    sVar2 = (in_A0[2] >> 4 & 0x7fe0) * 2 + (*in_A0 >> 9);
    psVar8 = (short *)((int)in_A0 + (short)in_A0[0x5d] + 0xbc);
    if (sVar2 == *psVar8) {
      if (psVar8[1] == -1) {
        if ((in_A0[0x58] & 0x800) != 0) {
          FUN_00014902();
          uVar3 = *(ushort *)(extraout_A0_06 + 0x34) >> 0xd & 6;
          if (((&DAT_00ff1700)[(short)(*(short *)(&DAT_000148ca + (short)uVar3) + sVar2)] == '\0')
             && ((&DAT_00ff1700)
                 [(short)(*(short *)(&DAT_000148ca + (short)(uVar3 + 2 & 6)) +
                         *(short *)(&DAT_000148ca + (short)(uVar3 + 4 & 6)) + sVar2)] == '\0')) {
            FUN_000148aa();
            sVar5 = extraout_D1w;
            iVar7 = extraout_A0_07;
          }
          else {
            sVar4 = 4;
            while (FUN_00014764(), sVar5 = extraout_D1w_00, iVar7 = extraout_A0_08,
                  extraout_D1w_00 == sVar2) {
              sVar4 = sVar4 + -1;
              if (sVar4 == -1) {
                FUN_00014756();
                return;
              }
            }
          }
          *(undefined2 *)(iVar7 + 0xba) = 0;
          *(short *)(iVar7 + 0xbc) = sVar5;
          *(undefined2 *)(iVar7 + 0xbe) = 0xffff;
          return;
        }
        *(byte *)(in_A0 + 0x58) = *(byte *)(in_A0 + 0x58) & 0xfd;
        *(byte *)(in_A0 + 0x58) = *(byte *)(in_A0 + 0x58) & 0xf7;
        FUN_00014c2e();
        in_A0 = extraout_A0_05;
        goto LAB_000145e8;
      }
      in_A0[0x5d] = in_A0[0x5d] + 2;
    }
    FUN_00014b4e();
    uVar6 = unaff_D2w << 0xd | unaff_D2w >> 3;
    uVar3 = FUN_00015ca4();
    iVar7 = extraout_A0_04;
  }
  *(ushort *)(iVar7 + 0x34) = uVar6;
  *(ushort *)(iVar7 + 200) = uVar3;
  *(undefined1 **)(iVar7 + 0x3a) = &LAB_0000b574;
  return;
}


