// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00000236


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void ResetEntry(void)

{
  char cVar1;
  byte bVar2;
  short *psVar3;
  undefined4 *puVar4;
  short *psVar5;
  short sVar6;
  short sVar7;
  byte bVar8;
  uint uVar9;
  short sVar10;
  short sVar11;
  undefined1 *puVar12;
  char *pcVar13;
  byte *pbVar14;
  char *extraout_A1;
  undefined2 *puVar16;
  undefined4 *puVar17;
  undefined4 *puVar18;
  undefined4 *puVar19;
  byte *pbVar15;
  
  psVar5 = DAT_000002da;
  puVar4 = DAT_000002d6;
  psVar3 = DAT_000002d2;
  pbVar14 = DAT_000002ce;
  puVar12 = DAT_000002ca;
  sVar7 = DAT_000002c8;
  sVar11 = DAT_000002c6;
  if (_DAT_00a10008 == 0 && _DAT_00a1000c == 0) {
    uVar9 = (uint)DAT_000002c4;
    if ((DAT_000002ce[-0x10ff] & 0xf) != 0) {
      pbVar14[0x2f00] = 0x53;
      pbVar14[0x2f01] = 0x45;
      pbVar14[0x2f02] = 0x47;
      pbVar14[0x2f03] = 0x41;
    }
    puVar19 = (undefined4 *)0x0;
    sVar6 = 0x17;
    puVar18 = (undefined4 *)&DAT_000002de;
    do {
      puVar17 = puVar18;
      sVar10 = (short)CONCAT31((int3)(uVar9 >> 8),*(undefined1 *)puVar17);
      *psVar5 = sVar10;
      uVar9 = (uint)(ushort)(sVar7 + sVar10);
      sVar6 = sVar6 + -1;
      puVar18 = (undefined4 *)((int)puVar17 + 1);
    } while (sVar6 != -1);
    *(undefined4 *)psVar5 = *(undefined4 *)((int)puVar17 + 1);
    *(undefined2 *)puVar4 = 0;
    *(short *)pbVar14 = sVar7;
    *psVar3 = sVar7;
    do {
    } while ((*pbVar14 & 1) != 0);
    sVar6 = 0x25;
    puVar18 = (undefined4 *)((int)puVar17 + 5);
    do {
      puVar17 = puVar18;
      *puVar12 = *(undefined1 *)puVar17;
      sVar6 = sVar6 + -1;
      puVar12 = puVar12 + 1;
      puVar18 = (undefined4 *)((int)puVar17 + 1);
    } while (sVar6 != -1);
    *psVar3 = 0;
    pbVar14[0] = 0;
    pbVar14[1] = 0;
    *psVar3 = sVar7;
    do {
      puVar19 = puVar19 + -1;
      *puVar19 = 0;
      sVar11 = sVar11 + -1;
    } while (sVar11 != -1);
    *(undefined4 *)psVar5 = *(undefined4 *)((int)puVar17 + 1);
    *(undefined4 *)psVar5 = *(undefined4 *)((int)puVar17 + 5);
    sVar11 = 0x1f;
    do {
      *puVar4 = 0;
      sVar11 = sVar11 + -1;
    } while (sVar11 != -1);
    *(undefined4 *)psVar5 = *(undefined4 *)((int)puVar17 + 9);
    sVar11 = 0x13;
    do {
      *puVar4 = 0;
      sVar11 = sVar11 + -1;
    } while (sVar11 != -1);
    sVar11 = 3;
    puVar12 = (undefined1 *)((int)puVar17 + 0xd);
    do {
      *(undefined1 *)((int)puVar4 + 0x11) = *puVar12;
      sVar11 = sVar11 + -1;
      puVar12 = puVar12 + 1;
    } while (sVar11 != -1);
    *psVar3 = 0;
  }
  psVar5 = DAT_000002da;
  puVar4 = DAT_000002d6;
  psVar3 = DAT_000002d2;
  pbVar14 = DAT_000002ce;
  sVar11 = DAT_000002c8;
  cVar1 = *(char *)((short)(ushort)(DAT_00a10001 >> 6) + 0x46c);
  if (cVar1 != '\0') {
    pcVar13 = &DAT_000001f0;
    sVar7 = 0xf;
    do {
      if (cVar1 == *pcVar13) {
        uVar9 = (uint)DAT_000002c4;
        sVar7 = 0x17;
        puVar18 = (undefined4 *)&DAT_000002de;
        do {
          puVar19 = (undefined4 *)((int)puVar18 + 1);
          sVar6 = (short)CONCAT31((int3)(uVar9 >> 8),*(undefined1 *)puVar18);
          *psVar5 = sVar6;
          uVar9 = (uint)(ushort)(sVar11 + sVar6);
          sVar7 = sVar7 + -1;
          puVar18 = puVar19;
        } while (sVar7 != -1);
        *(undefined4 *)psVar5 = *puVar19;
        *(undefined2 *)puVar4 = 0;
        sVar7 = 0x3ffe;
        do {
          sVar7 = sVar7 + -1;
        } while (sVar7 != -1);
        *(short *)pbVar14 = sVar11;
        *psVar3 = sVar11;
        *(undefined4 *)psVar5 = DAT_00000320;
        *(undefined4 *)psVar5 = DAT_00000324;
        sVar11 = 0x1f;
        do {
          *puVar4 = 0;
          sVar11 = sVar11 + -1;
        } while (sVar11 != -1);
        *(undefined4 *)psVar5 = DAT_00000328;
        sVar11 = 0x13;
        do {
          *puVar4 = 0;
          sVar11 = sVar11 + -1;
        } while (sVar11 != -1);
        sVar11 = 3;
        puVar12 = &DAT_0000032c;
        do {
          *(undefined1 *)((int)puVar4 + 0x11) = *puVar12;
          sVar11 = sVar11 + -1;
          puVar12 = puVar12 + 1;
        } while (sVar11 != -1);
        FUN_0000073a();
        FUN_0000075e();
        return;
      }
      pcVar13 = pcVar13 + 1;
      sVar7 = sVar7 + -1;
    } while (sVar7 != -1);
    _DAT_00c00004 = 0x40000000;
    pbVar14 = (byte *)(s_PAL_AND_FRENCH_SECAM_MEGA_DRIVE_000004ce + 0x20);
    sVar11 = 0x3a;
    uVar9 = 0x10000000;
    do {
      sVar7 = 7;
      pbVar15 = pbVar14;
      do {
        pbVar14 = pbVar15 + 1;
        bVar8 = *pbVar15;
        _DAT_00c00000 = 0;
        sVar6 = 7;
        do {
          uVar9 = uVar9 << 4 | uVar9 >> 0x1c;
          bVar2 = bVar8 << 7;
          bVar8 = bVar2 | bVar8 >> 1;
          if (bVar2 != 0) {
            _DAT_00c00000 = uVar9 | _DAT_00c00000;
          }
          sVar6 = sVar6 + -1;
        } while (sVar6 != -1);
        sVar7 = sVar7 + -1;
        pbVar15 = pbVar14;
      } while (sVar7 != -1);
      sVar11 = sVar11 + -1;
    } while (sVar11 != -1);
    FUN_00000436();
    for (pcVar13 = &DAT_000001f0; *pcVar13 != ' '; pcVar13 = pcVar13 + 1) {
      for (puVar16 = &DAT_00000470; (char)*puVar16 != '\0'; puVar16 = puVar16 + 3) {
        if ((char)*puVar16 == *pcVar13) {
          if ((pcVar13[1] == ' ') && (pcVar13 != &DAT_000001f0)) {
            FUN_00000436();
          }
          FUN_00000436();
          pcVar13 = extraout_A1;
          break;
        }
      }
    }
    FUN_00000436();
  }
  do {
                    /* WARNING: Do nothing block with infinite loop */
  } while( true );
}


