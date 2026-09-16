// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0001D33A


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0001d33a(void)

{
  ushort uVar1;
  short sVar2;
  ushort uVar3;
  ushort uVar4;
  ushort uVar5;
  short sVar6;
  short extraout_D1w;
  ushort uVar7;
  short sVar8;
  ushort uVar9;
  short sVar10;
  int iVar11;
  uint *extraout_A0;
  int in_A1;
  int extraout_A1;
  int unaff_A2;
  undefined1 *puVar12;
  undefined1 *puVar13;
  undefined1 *puVar15;
  bool bVar16;
  short *psVar17;
  short *psVar18;
  undefined1 *puVar14;
  
  uVar1 = _DAT_00ffdf72 + *(short *)(unaff_A2 + 2);
  sVar2 = (*(ushort *)(unaff_A2 + 4) & 3) + 1;
  uVar9 = sVar2 * 8;
  sVar10 = 0x1f;
  uVar7 = _DAT_00ffdf70;
  puVar12 = (undefined1 *)register0x0000003c;
  do {
    uVar4 = uVar7 - uVar1;
    puVar15 = puVar12;
    if (((uVar1 <= uVar7) && (uVar9 != uVar4)) && (uVar4 <= uVar9)) {
      uVar3 = *(ushort *)(unaff_A2 + 6) & 0x7ff;
      uVar5 = uVar3 * 0x20;
      iVar11 = (short)((uVar3 - _DAT_00ffdf74) * 0x20) + in_A1;
      uVar3 = *(ushort *)(unaff_A2 + 4) >> 2 & 3;
      *(ushort *)(puVar12 + -2) = uVar3;
      sVar8 = sVar2 * 0x20;
      bVar16 = CARRY2(uVar4 * 4,uVar5);
      sVar6 = uVar4 * 4 + uVar5;
      do {
        *(short *)(puVar12 + -2) = sVar6;
        *(ushort *)(puVar12 + -4) =
             (ushort)(byte)(bVar16 << 4 | (sVar6 < 0) << 3 | (sVar6 == 0) << 2);
        *(undefined4 *)(puVar12 + -8) = 0x1d3ba;
        FUN_00000e9c();
        *(short *)(puVar12 + -6) = sVar10;
        psVar17 = (short *)(puVar12 + -10);
        *psVar17 = extraout_D1w;
        *(undefined4 *)(puVar12 + -0xe) = 0x1d3cc;
        _DAT_00c00000 = FUN_0001d3f4();
        _DAT_00c00000 = *extraout_A0 & _DAT_00c00000;
        sVar10 = *(short *)(puVar12 + -8);
        psVar18 = (short *)(puVar12 + -6);
        puVar13 = puVar12 + -4;
        puVar14 = puVar12 + -4;
        puVar12 = puVar12 + -4;
        sVar6 = sVar8 + *psVar18;
        bVar16 = (uint)((int)sVar8 + (int)extraout_A0) <
                 (uint)((short)((uVar3 + 1) * 4 * uVar9) + iVar11);
        in_A1 = extraout_A1;
        puVar15 = puVar13;
        if (!bVar16) break;
        puVar15 = puVar14;
      } while (*psVar17 != 0);
    }
    uVar7 = uVar7 + 1;
    sVar10 = sVar10 + -1;
    puVar12 = puVar15;
    if (sVar10 == -1) {
      return;
    }
  } while( true );
}


