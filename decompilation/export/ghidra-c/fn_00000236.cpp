// Ghidra headless export
// Entry: 0x00000236
// Name: reset_and_enter_boot
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000436 write_startup_text_to_vdp
//   0x0000073A wait_for_vdp_ready
//   0x0000075E run_game_loop


void reset_and_enter_boot(void)

{
  char cVar1;
  byte bVar2;
  undefined4 uVar3;
  int iVar4;
  word wVar5;
  undefined *puVar6;
  undefined *puVar7;
  undefined *puVar8;
  undefined *puVar9;
  short sVar10;
  short sVar11;
  byte bVar12;
  uint uVar13;
  uint uVar14;
  short sVar15;
  short sVar16;
  undefined *puVar17;
  char *pcVar18;
  byte *pbVar19;
  char *extraout_A1;
  undefined2 *puVar21;
  undefined4 *puVar22;
  undefined1 *puVar23;
  undefined4 *puVar24;
  undefined4 *puVar25;
  bool bVar26;
  byte *pbVar20;
  
  puVar9 = PTR_VDP_CTRL_000002da;
  puVar8 = PTR_VDP_DATA_000002d6;
  puVar7 = PTR_IO_Z80RES_000002d2;
  puVar6 = PTR_IO_Z80BUS_000002ce;
  puVar17 = PTR_DAT_000002ca;
  sVar11 = DAT_000002c8;
  sVar16 = DAT_000002c6;
  uVar3 = Ram00a10008;
  iVar4 = Ram00a10008;
  bVar26 = iVar4 == 0;
  if (bVar26) {
    wVar5 = IO_EXT_CTRL;
    wVar5 = IO_EXT_CTRL;
    bVar26 = wVar5 == 0;
  }
  if (bVar26) {
    uVar14 = (uint)DAT_000002c4;
    if ((PTR_IO_Z80BUS_000002ce[-0x10ff] & 0xf) != 0) {
      *(undefined4 *)(PTR_IO_Z80BUS_000002ce + 0x2f00) = 0x53454741;
    }
    puVar25 = (undefined4 *)0x0;
    sVar10 = 0x17;
    puVar24 = (undefined4 *)&DAT_000002de;
    do {
      puVar22 = puVar24;
      sVar15 = (short)CONCAT31((int3)(uVar14 >> 8),*(undefined1 *)puVar22);
      *(short *)puVar9 = sVar15;
      uVar14 = (uint)(ushort)(sVar11 + sVar15);
      sVar10 = sVar10 + -1;
      puVar24 = (undefined4 *)((int)puVar22 + 1);
    } while (sVar10 != -1);
    *(undefined4 *)puVar9 = *(undefined4 *)((int)puVar22 + 1);
    *(undefined2 *)puVar8 = 0;
    *(short *)puVar6 = sVar11;
    *(short *)puVar7 = sVar11;
    do {
    } while ((*puVar6 & 1) != 0);
    sVar10 = 0x25;
    puVar24 = (undefined4 *)((int)puVar22 + 5);
    do {
      puVar22 = puVar24;
      *puVar17 = *(undefined1 *)puVar22;
      sVar10 = sVar10 + -1;
      puVar17 = puVar17 + 1;
      puVar24 = (undefined4 *)((int)puVar22 + 1);
    } while (sVar10 != -1);
    *(undefined2 *)puVar7 = 0;
    *(undefined2 *)puVar6 = 0;
    *(short *)puVar7 = sVar11;
    do {
      puVar25 = puVar25 + -1;
      *puVar25 = 0;
      sVar16 = sVar16 + -1;
    } while (sVar16 != -1);
    *(undefined4 *)puVar9 = *(undefined4 *)((int)puVar22 + 1);
    *(undefined4 *)puVar9 = *(undefined4 *)((int)puVar22 + 5);
    sVar16 = 0x1f;
    do {
      *(undefined4 *)puVar8 = 0;
      sVar16 = sVar16 + -1;
    } while (sVar16 != -1);
    *(undefined4 *)puVar9 = *(undefined4 *)((int)puVar22 + 9);
    sVar16 = 0x13;
    do {
      *(undefined4 *)puVar8 = 0;
      sVar16 = sVar16 + -1;
    } while (sVar16 != -1);
    sVar16 = 3;
    puVar23 = (undefined1 *)((int)puVar22 + 0xd);
    do {
      puVar8[0x11] = *puVar23;
      sVar16 = sVar16 + -1;
      puVar23 = puVar23 + 1;
    } while (sVar16 != -1);
    *(undefined2 *)puVar7 = 0;
  }
  puVar8 = PTR_VDP_CTRL_000002da;
  puVar7 = PTR_VDP_DATA_000002d6;
  puVar6 = PTR_IO_Z80RES_000002d2;
  puVar17 = PTR_IO_Z80BUS_000002ce;
  sVar16 = DAT_000002c8;
  wVar5 = VDP_CTRL;
  wVar5 = VDP_CTRL;
  bVar12 = IO_PCBVER._1_1_;
  cVar1 = *(char *)((short)(ushort)(bVar12 >> 6) + 0x46c);
  if (cVar1 != '\0') {
    pcVar18 = GameHeader_00000100.Notes + 0x34;
    sVar11 = 0xf;
    do {
      if (cVar1 == *pcVar18) {
        uVar14 = (uint)DAT_000002c4;
        sVar11 = 0x17;
        puVar24 = (undefined4 *)&DAT_000002de;
        do {
          puVar25 = (undefined4 *)((int)puVar24 + 1);
          sVar10 = (short)CONCAT31((int3)(uVar14 >> 8),*(undefined1 *)puVar24);
          *(short *)puVar8 = sVar10;
          uVar14 = (uint)(ushort)(sVar16 + sVar10);
          sVar11 = sVar11 + -1;
          puVar24 = puVar25;
        } while (sVar11 != -1);
        *(undefined4 *)puVar8 = *puVar25;
        *(undefined2 *)puVar7 = 0;
        sVar11 = 0x3ffe;
        do {
          sVar11 = sVar11 + -1;
        } while (sVar11 != -1);
        *(short *)puVar17 = sVar16;
        *(short *)puVar6 = sVar16;
        *(undefined4 *)puVar8 = DAT_00000320;
        *(undefined4 *)puVar8 = DAT_00000324;
        sVar16 = 0x1f;
        do {
          *(undefined4 *)puVar7 = 0;
          sVar16 = sVar16 + -1;
        } while (sVar16 != -1);
        *(undefined4 *)puVar8 = DAT_00000328;
        sVar16 = 0x13;
        do {
          *(undefined4 *)puVar7 = 0;
          sVar16 = sVar16 + -1;
        } while (sVar16 != -1);
        sVar16 = 3;
        puVar23 = &DAT_0000032c;
        do {
          puVar7[0x11] = *puVar23;
          sVar16 = sVar16 + -1;
          puVar23 = puVar23 + 1;
        } while (sVar16 != -1);
        wait_for_vdp_ready();
        run_game_loop();
        return;
      }
      pcVar18 = pcVar18 + 1;
      sVar11 = sVar11 + -1;
    } while (sVar11 != -1);
    VDP_CTRL = 0x8164;
    VDP_CTRL = 0x8230;
    VDP_CTRL = 0x8c81;
    VDP_CTRL = 0x8f02;
    VDP_CTRL = 0x9001;
    Ram00c00004 = 0xc0020000;
    VDP_DATA = 0xeee;
    Ram00c00004 = 0x40000000;
    pbVar19 = &DAT_000004ee;
    sVar16 = 0x3a;
    uVar14 = 0x10000000;
    do {
      sVar11 = 7;
      pbVar20 = pbVar19;
      do {
        pbVar19 = pbVar20 + 1;
        bVar12 = *pbVar20;
        uVar13 = 0;
        sVar10 = 7;
        do {
          uVar14 = uVar14 << 4 | uVar14 >> 0x1c;
          bVar2 = bVar12 << 7;
          bVar12 = bVar2 | bVar12 >> 1;
          if (bVar2 != 0) {
            uVar13 = uVar14 | uVar13;
          }
          sVar10 = sVar10 + -1;
        } while (sVar10 != -1);
        Ram00c00000 = uVar13;
        sVar11 = sVar11 + -1;
        pbVar20 = pbVar19;
      } while (sVar11 != -1);
      sVar16 = sVar16 + -1;
    } while (sVar16 != -1);
    write_startup_text_to_vdp();
    for (pcVar18 = GameHeader_00000100.Notes + 0x34; *pcVar18 != ' '; pcVar18 = pcVar18 + 1) {
      for (puVar21 = &DAT_00000470; (char)*puVar21 != '\0'; puVar21 = puVar21 + 3) {
        if ((char)*puVar21 == *pcVar18) {
          if ((pcVar18[1] == ' ') && (pcVar18 != GameHeader_00000100.Notes + 0x34)) {
            write_startup_text_to_vdp();
          }
          write_startup_text_to_vdp();
          pcVar18 = extraout_A1;
          break;
        }
      }
    }
    write_startup_text_to_vdp();
  }
  do {
                    /* WARNING: Do nothing block with infinite loop */
  } while( true );
}


