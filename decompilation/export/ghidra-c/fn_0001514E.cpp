// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0001514E


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0001514e(void)

{
  uint uVar1;
  undefined2 uVar2;
  ushort uVar3;
  undefined4 extraout_D1;
  short sVar4;
  int in_A0;
  int extraout_A0;
  int extraout_A0_00;
  undefined2 *extraout_A1;
  undefined2 *extraout_A1_00;
  undefined2 *puVar5;
  undefined2 *extraout_A1_01;
  undefined2 *puVar6;
  undefined *puVar7;
  byte *pbVar8;
  bool bVar9;
  undefined1 uVar10;
  
  _DAT_00ffdd3a = 0;
  if (((*(char *)(in_A0 + 0x57) == '\b') || (*(char *)(in_A0 + 0x57) == '\t')) ||
     (*(char *)(in_A0 + 0x57) == '\n')) {
    _DAT_00ffdd3a = -1;
  }
  if (DAT_00ffe186 != '\x03') {
    uVar3 = *(ushort *)(in_A0 + 0x16) & 0x7ff;
    uVar10 = uVar3 < 0x3c;
    bVar9 = uVar3 == 0x3c;
    if (uVar3 < 0x3d) {
      FUN_000038ac();
      if ((!bVar9) && (FUN_00015a8c(), !(bool)uVar10)) {
        FUN_0001dbaa();
        FUN_00014ff8();
        *(undefined1 *)((int)extraout_A1 + 0x89) = 0x7f;
        *(undefined1 *)((int)extraout_A1 + 0x8b) = 0;
        uVar10 = 0;
        puVar5 = extraout_A1;
        if (_DAT_00ffdd3a != 0) {
          FUN_000152fa();
          puVar5 = extraout_A1_00;
        }
        uVar1 = FUN_00015a8c();
        if (!(bool)uVar10) {
          DAT_00ffdd6d = 0;
          sVar4 = (short)CONCAT31((int3)(uVar1 >> 8),*(char *)(extraout_A0 + 0x56) + 'n');
          if (((*(byte *)(extraout_A0 + sVar4) & 2) != 0) ||
             ((*(byte *)(extraout_A0 + sVar4) & 4) != 0)) {
            DAT_00ffdd6d = DAT_ffffffff;
          }
          FUN_000530de();
          uVar2 = (undefined2)uVar1;
          if (((_DAT_00ffdf40 == 2) &&
              (pbVar8 = (byte *)((short)*(char *)(extraout_A0_00 + 0x56) + extraout_A0_00 + 0x6e),
              (*pbVar8 & 2) == 0)) && ((*pbVar8 & 4) == 0)) {
            FUN_0001e04a(uVar1 & 0xffff,extraout_D1);
          }
          sVar4 = 0x4f;
          puVar6 = extraout_A1_01;
          do {
            *puVar6 = *puVar5;
            sVar4 = sVar4 + -1;
            puVar6 = puVar6 + 1;
            puVar5 = puVar5 + 1;
          } while (sVar4 != -1);
          *extraout_A1_01 = _DAT_00ffdd60;
          extraout_A1_01[2] = _DAT_00ffdd62;
          extraout_A1_01[0x1f] = uVar2;
          *(byte *)((int)extraout_A1_01 + 0x11) = *(byte *)((int)extraout_A1_01 + 0x11) | 1;
          extraout_A1_01[7] = 0;
          *(undefined1 **)(extraout_A1_01 + 0x1d) = &LAB_0000ac4e;
          *(undefined1 *)(extraout_A1_01 + 0x32) = DAT_00ffdd6d;
          *(undefined1 *)((int)extraout_A1_01 + 0x41) = 0;
          puVar7 = &DAT_000062ec;
          if (_DAT_00ffdd3a != 0) {
            puVar7 = &DAT_000062fc;
          }
          uVar3 = (ushort)extraout_A1_01[0x17] >> 0xd;
          if ((extraout_A1_01[0x17] & 0x1000) != 0) {
            uVar3 = uVar3 + 1;
          }
          extraout_A1_01[0xb] = *(undefined2 *)(puVar7 + (short)((uVar3 & 7) << 1));
          return;
        }
      }
    }
    else if ((*(byte *)(in_A0 + 0x10) & 2) != 0) {
      _DAT_00ffdfe6 = 0xffff;
    }
  }
  return;
}


