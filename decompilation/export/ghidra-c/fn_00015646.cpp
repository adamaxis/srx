// Ghidra headless export
// Entry: 0x00015646
// Name: FUN_00015646
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x000036FC FUN_000036fc
//   0x00015118 FUN_00015118
//   0x00015A8C find_first_available_entity_slot
//   0x00015D3C FUN_00015d3c
//   0x0001A2CC FUN_0001a2cc
//   0x0001DBAA FUN_0001dbaa
//   0x00055848 FUN_00055848


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00015646(void)

{
  short *psVar1;
  short sVar2;
  ushort uVar3;
  short sVar4;
  undefined2 uVar5;
  ushort uVar6;
  short sVar7;
  short unaff_D5w;
  int in_A0;
  undefined2 *extraout_A0;
  int extraout_A0_00;
  undefined2 *extraout_A1;
  short *extraout_A1_00;
  int extraout_A1_01;
  short unaff_A2w;
  undefined1 uVar8;
  bool bVar9;
  undefined6 uVar10;
  
  if ((*(ushort *)(in_A0 + 0x16) & 0x7ff) < 0x3d) {
    uVar8 = 0;
    FUN_000036fc();
    find_first_available_entity_slot();
    if (!(bool)uVar8) {
      FUN_0001dbaa();
      uVar5 = FUN_00015118();
      *(undefined1 **)(extraout_A1 + 0x1d) = &LAB_00015390;
      extraout_A1[0x16] = 0x2000;
      *extraout_A1 = *extraout_A0;
      extraout_A1[2] = extraout_A0[2];
      extraout_A1[0x17] = extraout_A0[0x17];
      *(undefined1 *)((int)extraout_A1 + 0x9d) = *(undefined1 *)((int)extraout_A0 + 0x9d);
      *(undefined1 *)((int)extraout_A1 + 0x57) = *(undefined1 *)((int)extraout_A0 + 0x57);
      *(undefined1 *)(extraout_A1 + 0x2b) = *(undefined1 *)(extraout_A0 + 0x2b);
      extraout_A1[0x1f] = uVar5;
      extraout_A1[0x23] = 0x13;
      extraout_A1[0x1b] = 0xfb00;
      extraout_A1[0x1a] = 0xf400;
      extraout_A1[7] = 2;
      extraout_A1[0x20] = 0xb4;
      extraout_A1[0xb] = 0;
      bVar9 = false;
      uVar10 = FUN_0001a2cc();
      if (!bVar9) {
        extraout_A1_00[0x18] = (short)((uint6)uVar10 >> 0x20);
        extraout_A1_00[0x10] = (short)uVar10;
        extraout_A1_00[0x13] = unaff_A2w;
        uVar3 = extraout_A1_00[0x17];
        uVar6 = uVar3 << 3 | uVar3 >> 0xd;
        if ((short)uVar6 < 0) {
          uVar6 = uVar6 + 1;
        }
        sVar7 = (uVar6 & 7) * 4;
        DAT_00ffdd60 = *extraout_A1_00 + *(short *)(&DAT_00015a6c + sVar7);
        DAT_00ffdd62 = extraout_A1_00[2] + *(short *)(&DAT_00015a6c + (short)(sVar7 + 2));
        *extraout_A1_00 = *(short *)(&DAT_00015a4c + sVar7) + *extraout_A1_00;
        sVar7 = *(short *)(&DAT_00015a4c + (short)(sVar7 + 2));
        extraout_A1_00[2] = sVar7 + extraout_A1_00[2];
        psVar1 = *(short **)(extraout_A0_00 + 0xa2);
        if (psVar1 == (short *)0x0) {
          *(byte *)((int)extraout_A1_00 + 0x9f) = *(byte *)(extraout_A0_00 + 0x87) >> 1;
          uVar3 = uVar3 >> 8 & 0xfffe;
          sVar7 = *(short *)((int)&DAT_00015e16 + (int)(short)uVar3);
          extraout_A1_00[0x26] =
               (short)(*(short *)((int)&DAT_00015e56 + (int)(short)uVar3) * 8 >> 5);
          extraout_A1_00[0x27] = (short)(sVar7 * 8 >> 5);
          return;
        }
        sVar2 = *psVar1;
        sVar4 = psVar1[2];
        FUN_00055848(psVar1,uVar3);
        *(undefined1 *)((int)extraout_A1_00 + 0x9f) = DAT_00fff0dd;
        uVar3 = unaff_D5w * 0x10 + sVar4;
        uVar6 = sVar7 * 0x10 + sVar2;
        FUN_00015d3c();
        if (0xbdf < uVar6) {
          uVar6 = 0xbe0;
        }
        *(ushort *)(extraout_A1_01 + 0x2e) = uVar3;
        *(ushort *)(extraout_A1_01 + 0x40) = (uVar6 / 0x80) * 2;
        uVar3 = uVar3 >> 8 & 0xfffe;
        sVar7 = *(short *)((int)&DAT_00015e16 + (int)(short)uVar3);
        *(short *)(extraout_A1_01 + 0x4c) =
             (short)(*(short *)((int)&DAT_00015e56 + (int)(short)uVar3) * 8 >> 4);
        *(short *)(extraout_A1_01 + 0x4e) = (short)(sVar7 * 8 >> 4);
        *(ushort *)(extraout_A1_01 + 0x36) = (uVar6 / 0x80) * -0x20;
        return;
      }
      *(undefined1 *)(extraout_A1_00 + 8) = 0;
    }
  }
  return;
}


