// Ghidra headless export
// Entry: 0x0001D33A
// Name: FUN_0001d33a
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000E9C FUN_00000e9c
//   0x0001D3F4 FUN_0001d3f4


void FUN_0001d33a(void)

{
  ushort uVar1;
  short sVar2;
  ushort uVar3;
  ushort uVar4;
  ushort uVar6;
  short sVar7;
  uint uVar5;
  short extraout_D1w;
  ushort uVar8;
  short sVar9;
  ushort uVar10;
  short sVar11;
  int iVar12;
  uint *extraout_A0;
  int in_A1;
  int extraout_A1;
  int unaff_A2;
  undefined1 *puVar13;
  undefined1 *puVar14;
  undefined1 *puVar16;
  bool bVar17;
  short *psVar18;
  short *psVar19;
  undefined1 *puVar15;
  
  uVar1 = DAT_00ffdf72 + *(short *)(unaff_A2 + 2);
  sVar2 = (*(ushort *)(unaff_A2 + 4) & 3) + 1;
  uVar10 = sVar2 * 8;
  sVar11 = 0x1f;
  uVar8 = DAT_00ffdf70;
  puVar13 = (undefined1 *)register0x0000003c;
  do {
    uVar4 = uVar8 - uVar1;
    puVar16 = puVar13;
    if (((uVar1 <= uVar8) && (uVar10 != uVar4)) && (uVar4 <= uVar10)) {
      uVar3 = *(ushort *)(unaff_A2 + 6) & 0x7ff;
      uVar6 = uVar3 * 0x20;
      iVar12 = (short)((uVar3 - DAT_00ffdf74) * 0x20) + in_A1;
      uVar3 = *(ushort *)(unaff_A2 + 4) >> 2 & 3;
      *(ushort *)(puVar13 + -2) = uVar3;
      sVar9 = sVar2 * 0x20;
      bVar17 = CARRY2(uVar4 * 4,uVar6);
      sVar7 = uVar4 * 4 + uVar6;
      do {
        *(short *)(puVar13 + -2) = sVar7;
        *(ushort *)(puVar13 + -4) =
             (ushort)(byte)(bVar17 << 4 | (sVar7 < 0) << 3 | (sVar7 == 0) << 2);
        *(undefined4 *)(puVar13 + -8) = 0x1d3ba;
        FUN_00000e9c();
        *(short *)(puVar13 + -6) = sVar11;
        psVar18 = (short *)(puVar13 + -10);
        *psVar18 = extraout_D1w;
        *(undefined4 *)(puVar13 + -0xe) = 0x1d3cc;
        uVar5 = FUN_0001d3f4();
        Ram00c00000 = *extraout_A0 & uVar5;
        sVar11 = *(short *)(puVar13 + -8);
        psVar19 = (short *)(puVar13 + -6);
        puVar14 = puVar13 + -4;
        puVar15 = puVar13 + -4;
        puVar13 = puVar13 + -4;
        sVar7 = sVar9 + *psVar19;
        bVar17 = (uint)((int)sVar9 + (int)extraout_A0) <
                 (uint)((short)((uVar3 + 1) * 4 * uVar10) + iVar12);
        in_A1 = extraout_A1;
        puVar16 = puVar14;
        if (!bVar17) break;
        puVar16 = puVar15;
      } while (*psVar18 != 0);
    }
    uVar8 = uVar8 + 1;
    sVar11 = sVar11 + -1;
    puVar13 = puVar16;
    if (sVar11 == -1) {
      return;
    }
  } while( true );
}


