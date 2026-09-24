// Ghidra headless export
// Entry: 0x00011740
// Name: FUN_00011740
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000BD2 wait_for_vblank
//   0x00000BF2 latch_controller_events
//   0x00000D94 wait_for_controller_event
//   0x00000FFE FUN_00000ffe
//   0x0000102A FUN_0000102a
//   0x000011C0 FUN_000011c0
//   0x00001358 copy_source_word_block
//   0x00001530 decode_and_upload_source_stream
//   0x00001728 render_hud_sprite_stream
//   0x0000195C sample_controller_input
//   0x0000334C FUN_0000334c
//   0x00003404 FUN_00003404
//   0x00003674 FUN_00003674
//   0x000036FC FUN_000036fc
//   0x000037EC FUN_000037ec
//   0x000038FA FUN_000038fa
//   0x0000391A FUN_0000391a
//   0x000039F2 FUN_000039f2
//   0x0000C0DA render_title_text_source
//   0x0000C216 FUN_0000c216
//   0x0000C376 move_wrapped_menu_cursor
//   0x0000C3E6 FUN_0000c3e6
//   0x0000C3F8 FUN_0000c3f8
//   0x0000C436 FUN_0000c436
//   0x0000D4C8 FUN_0000d4c8
//   0x0000D53C FUN_0000d53c
//   0x0000D5DC FUN_0000d5dc
//   0x0000D82A FUN_0000d82a
//   0x0000E660 FUN_0000e660
//   0x0000FDFE FUN_0000fdfe
//   0x0000FEBE move_selection_column
//   0x0000FF06 move_selection_row
//   0x0000FF5A FUN_0000ff5a
//   0x00010276 FUN_00010276
//   0x000106DA FUN_000106da
//   0x00010A7E FUN_00010a7e
//   0x00010C96 FUN_00010c96
//   0x000110E8 FUN_000110e8
//   0x000110EC FUN_000110ec
//   0x00011190 FUN_00011190
//   0x000111D0 FUN_000111d0
//   0x00011454 FUN_00011454
//   0x00011624 FUN_00011624
//   0x0001170C FUN_0001170c
//   0x00011712 FUN_00011712
//   0x000118C6 cycle_inventory_action_selector
//   0x00011900 FUN_00011900
//   0x00011928 FUN_00011928
//   0x00012148 FUN_00012148
//   0x0001253E FUN_0001253e
//   0x000125FA FUN_000125fa
//   0x0001275C FUN_0001275c
//   0x000127B0 FUN_000127b0
//   0x000129FE FUN_000129fe
//   0x00012C1E FUN_00012c1e
//   0x00012C36 FUN_00012c36
//   0x00012C42 FUN_00012c42
//   0x00012C5A FUN_00012c5a
//   0x000530DE FUN_000530de
//   0x000566E2 FUN_000566e2


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00011740(void)

{
  byte bVar1;
  byte bVar2;
  int iVar3;
  undefined2 *puVar4;
  char cVar7;
  undefined2 uVar5;
  char cVar8;
  short sVar6;
  undefined1 extraout_D1b;
  undefined1 extraout_D1b_00;
  undefined4 unaff_D3;
  uint uVar9;
  ushort uVar10;
  short sVar11;
  short unaff_D7w;
  byte *pbVar12;
  int extraout_A0;
  int extraout_A0_00;
  int extraout_A0_01;
  int iVar13;
  int extraout_A0_02;
  int extraout_A0_03;
  int extraout_A0_04;
  int extraout_A0_05;
  undefined2 *puVar14;
  undefined2 *extraout_A0_06;
  int extraout_A0_07;
  undefined4 *puVar15;
  int extraout_A1;
  int extraout_A1_00;
  int extraout_A1_01;
  undefined4 *puVar16;
  undefined2 *puVar17;
  bool bVar18;
  undefined1 uVar19;
  undefined2 uVar20;
  undefined2 local_2;
  
  uVar20 = _DAT_00fff0d0;
  uVar19 = DAT_00fff0cd;
  DAT_00fff0cd = 0;
  cVar7 = (char)unaff_D7w;
                    /* WARNING (jumptable): Sanity check requires truncation of jumptable */
                    /* WARNING: Could not find normalized switch variable to match jumptable */
  switch(DAT_00fff0d0) {
  case 2:
    FUN_00012c36();
    FUN_00011900();
    render_title_text_source();
    FUN_0000c216();
    uVar20 = _DAT_00fff0d0;
    _DAT_00fff0d0 = CONCAT11(2,DAT_00fff0d1);
    FUN_000111d0();
    _DAT_00fff116 = 0xffff;
    FUN_00011454();
    FUN_00011712();
    do {
      while( true ) {
        wait_for_vblank();
        latch_controller_events();
        sample_controller_input();
        if ((DAT_00ffdd22._0_1_ & 4) == 0) break;
        uVar5 = 0xffff;
LAB_00011be6:
        FUN_000530de(uVar5);
        FUN_000111d0();
        cycle_inventory_action_selector();
        _DAT_00fff116 = 0xffff;
        FUN_00011454();
        FUN_00011712();
      }
      if ((DAT_00ffdd22._0_1_ & 8) != 0) {
        uVar5 = 1;
        goto LAB_00011be6;
      }
    } while ((((DAT_00ffdd22._0_1_ & 0x40) == 0) && ((DAT_00ffdd22._0_1_ & 0x10) == 0)) &&
            ((DAT_00ffdd22._0_1_ & 0x20) == 0));
    cVar7 = '\0';
    uVar19 = 0;
    cVar8 = '\0';
    FUN_000530de();
    FUN_00012062();
    if ((bool)uVar19 || cVar8 != cVar7) goto LAB_00011c9c;
    _DAT_00fff0d0 = uVar20;
    if (DAT_00fff0d0 == '\x02') {
      FUN_00012c36();
      if ((-1 < (char)(*(char *)(DAT_00fffc04 + 0x2b) << 1)) &&
         ((char)(*(char *)((int)DAT_00fffc04 + 0x57) + -0xd) < '\0')) {
        if (*(char *)(DAT_00fffc04 + 0x27) != '\0') {
          DAT_00fff114 = '\0';
          FUN_000111d0();
          FUN_000127b0();
          render_title_text_source();
          do {
            while( true ) {
              wait_for_vblank();
              latch_controller_events();
              sample_controller_input();
              if (((DAT_00ffdd22._0_1_ & 4) == 0) && ((DAT_00ffdd22._0_1_ & 8) == 0)) break;
              FUN_000530de();
              FUN_000127b0();
            }
          } while ((DAT_00ffdd22._0_1_ & 0x40) == 0);
          FUN_000530de();
          if (DAT_00fff114 < '\0') {
            return;
          }
        }
        puVar17 = DAT_00fffc04;
        bVar1 = *(byte *)((int)DAT_00fffc04 + 0x4f) - 1;
        *(byte *)((int)DAT_00fffc04 + 0x4f) = bVar1;
        if ((int)((uint)bVar1 << 0x18) < 0) {
          if (*(char *)((int)DAT_00fffc04 + 0x4f) < '\x01') {
            FUN_00012c36();
            render_title_text_source();
            render_title_text_source();
            *(undefined1 *)(extraout_A0 + 0x4f) = 0;
          }
          return;
        }
        if (*(char *)(puVar17 + 0x2b) < '\0') {
          *(undefined1 *)(puVar17 + 0x2b) = 0x7f;
          *(undefined1 *)((int)puVar17 + 0x57) = 0;
          FUN_00003404();
        }
        FUN_000036fc();
        FUN_00012c36();
      }
      render_title_text_source();
      goto LAB_00011cb6;
    }
    if (DAT_00fff0d0 == '\x03') {
      FUN_00012c36();
      sVar11 = 2;
      puVar17 = &DAT_00ff0100;
      puVar16 = (undefined4 *)&DAT_00ffdd48;
      do {
        puVar15 = puVar16;
        if (((*(byte *)(puVar17 + 8) & 2) == 0) && ((*(byte *)(puVar17 + 8) & 1) != 0)) {
          puVar15 = puVar16 + 1;
          *puVar16 = puVar17;
        }
        puVar17 = puVar17 + 0x80;
        sVar11 = sVar11 + -1;
        puVar16 = puVar15;
      } while (sVar11 != -1);
      *puVar15 = 0xffffffff;
      if (_DAT_00ffdd48 < 0) goto LAB_00011e14;
      DAT_00ffdd28 = 0;
      render_title_text_source();
      _DAT_00ffdd2a = (uint)*(byte *)((int)DAT_00fffc04 + 0x4f);
      FUN_0000c216();
      FUN_0001209a();
      uVar20 = _DAT_00fff0d0;
      _DAT_00fff0d0 = CONCAT11(2,DAT_00fff0d1);
      FUN_000111d0();
      _DAT_00fff116 = 0xffff;
      FUN_00011454();
      FUN_00011712();
LAB_00011ea4:
      do {
        while( true ) {
          wait_for_vblank();
          latch_controller_events();
          sample_controller_input();
          if ((DAT_00ffdd22._0_1_ & 4) == 0) break;
          uVar5 = 0xffff;
LAB_00011ec4:
          FUN_000530de(uVar5);
          FUN_000111d0();
          cycle_inventory_action_selector();
          _DAT_00fff116 = 0xffff;
          FUN_00011454();
          FUN_00011712();
        }
        if ((DAT_00ffdd22._0_1_ & 8) != 0) {
          uVar5 = 1;
          goto LAB_00011ec4;
        }
        if ((((DAT_00ffdd22._0_1_ & 0x40) != 0) || ((DAT_00ffdd22._0_1_ & 0x10) != 0)) ||
           ((DAT_00ffdd22._0_1_ & 0x20) != 0)) {
          FUN_000530de();
          if (DAT_00fff0d0 == '\x02') {
            FUN_000120e4();
          }
          else if (DAT_00fff0d0 == '\x03') {
            FUN_000120e4();
          }
          else {
            if (DAT_00fff0d0 == '\x04') {
              iVar13 = DAT_00ffdd4c;
              if (DAT_00ffdd28 != 0) {
                iVar13 = _DAT_00ffdd48;
              }
              cVar7 = (char)_DAT_00ffdd2a;
              _DAT_00fff0d0 = uVar20;
              *(char *)((int)DAT_00fffc04 + 0x4f) = *(char *)((int)DAT_00fffc04 + 0x4f) - cVar7;
              *(char *)(iVar13 + 0x4f) = cVar7 + *(char *)(iVar13 + 0x4f);
              FUN_00012c36();
              render_title_text_source();
              goto LAB_00011cb6;
            }
            FUN_0001209a();
          }
          goto LAB_00011ea4;
        }
      } while ((DAT_00ffdd22._0_1_ & 0x80) == 0);
    }
    else if (DAT_00fff0d0 == '\x04') {
      FUN_00012c36();
      uVar9 = 0;
      uVar10 = 0;
      sVar11 = 2;
      puVar17 = &DAT_00ff0100;
      do {
        if ((*(byte *)(puVar17 + 8) & 1) != 0) {
          uVar10 = uVar10 + 1;
          uVar9 = (uint)(byte)(*(char *)((int)puVar17 + 0x4f) + (char)uVar9);
        }
        puVar17 = puVar17 + 0x80;
        sVar11 = sVar11 + -1;
      } while (sVar11 != -1);
      if (-1 < (short)(uVar10 - 2)) {
        if ((short)uVar9 != 0) {
          sVar11 = 2;
          puVar17 = &DAT_00ff0100;
          do {
            *(char *)((int)puVar17 + 0x4f) = (char)(uVar9 / uVar10);
            if ((*(byte *)(puVar17 + 8) & 2) != 0) {
              *(char *)((int)puVar17 + 0x4f) =
                   (char)(uVar9 % (uint)uVar10) + *(char *)((int)puVar17 + 0x4f);
            }
            puVar17 = puVar17 + 0x80;
            sVar11 = sVar11 + -1;
          } while (sVar11 != -1);
        }
        FUN_00012c36();
        render_title_text_source();
        goto LAB_00011cb6;
      }
LAB_00011e14:
      render_title_text_source();
      goto LAB_00011cb6;
    }
LAB_00011c9c:
    _DAT_00fff0d0 = uVar20;
    FUN_00012c36();
    render_title_text_source();
LAB_00011cb6:
    FUN_000111d0();
    wait_for_controller_event();
    FUN_00011928();
    return;
  case 3:
    _DAT_00fff116 = -1;
    VDP_CTRL = 0x9100;
    FUN_00000ffe();
    FUN_0000102a();
    FUN_00010c96();
    DAT_00ffd8d4 = 0x2c2;
    DAT_00ffd8f4 = 0x60;
    DAT_00ffef14 = DAT_00ffef14 | 1;
    sVar11 = 3;
    do {
      render_title_text_source();
      sVar11 = sVar11 + -1;
    } while (sVar11 != -1);
    sVar11 = 3;
    do {
      render_title_text_source();
      sVar11 = sVar11 + -1;
    } while (sVar11 != -1);
    sVar11 = 5;
    do {
      render_title_text_source();
      sVar11 = sVar11 + -1;
    } while (sVar11 != -1);
    sVar11 = 5;
    do {
      render_title_text_source();
      sVar11 = sVar11 + -1;
    } while (sVar11 != -1);
    _DAT_00fff0d0 = 0;
LAB_0000d0d6:
    FUN_000011c0();
    FUN_000011c0();
    FUN_000566e2();
    FUN_000110ec();
    render_title_text_source();
    render_title_text_source();
    render_title_text_source();
    sVar11 = 3;
    do {
      FUN_0000c216();
      sVar11 = sVar11 + -1;
    } while (sVar11 != -1);
    sVar11 = 1;
    do {
      FUN_0000c216();
      sVar11 = sVar11 + -1;
    } while (sVar11 != -1);
    FUN_0000c216();
    FUN_0000c216();
    FUN_0000c216();
    sVar11 = 5;
    do {
      FUN_0000c216();
      sVar11 = sVar11 + -1;
    } while (sVar11 != -1);
    sVar11 = 5;
    do {
      FUN_0000c216();
      sVar11 = sVar11 + -1;
    } while (sVar11 != -1);
LAB_0000d25e:
    do {
      FUN_0000d5dc();
      FUN_0000d82a();
      while( true ) {
        wait_for_vblank();
        latch_controller_events();
        sample_controller_input();
        if ((DAT_00ffdd22._0_1_ & 4) != 0) break;
        if ((DAT_00ffdd22._0_1_ & 8) != 0) {
          FUN_0000d5dc();
          bVar18 = DAT_00fff0d0 == '\0';
          if (bVar18) {
            _DAT_00fff0d0 = CONCAT11(1,DAT_00fff0d1);
          }
          goto LAB_0000d25e;
        }
        if ((DAT_00ffdd22._0_1_ & 1) != 0) {
          FUN_0000d5dc();
          bVar1 = DAT_00fff0d1 - 1;
          _DAT_00fff0d0 = CONCAT11(DAT_00fff0d0,bVar1);
          if ((int)((uint)bVar1 << 0x18) < 0) {
            _DAT_00fff0d0 = (ushort)DAT_00fff0d0 << 8;
          }
          goto LAB_0000d25e;
        }
        if ((DAT_00ffdd22._0_1_ & 2) != 0) {
          FUN_0000d5dc();
          uVar10 = _DAT_00fff0d0;
          cVar7 = DAT_00fff0d1 + '\x01';
          _DAT_00fff0d0 = CONCAT11(DAT_00fff0d0,cVar7);
          if ('\t' < cVar7) {
            _DAT_00fff0d0 = uVar10;
          }
          goto LAB_0000d25e;
        }
        if (((DAT_00ffdd22._0_1_ & 0x40) == 0) && ((DAT_00ffdd22._0_1_ & 0x10) == 0)) {
          if ((DAT_00ffdd22._0_1_ & 0x20) != 0) {
            FUN_0000d5dc();
            FUN_0000d4c8();
            goto LAB_0000d0d6;
          }
          if ((DAT_00ffdd22._0_1_ & 0x80) != 0) {
LAB_0000d4a0:
            VDP_CTRL = 0x9191;
            FUN_00000ffe();
            FUN_0000102a();
            _DAT_00fff116 = 0;
            FUN_0000334c();
            FUN_000039f2();
            return;
          }
        }
        else {
          if (_DAT_00fff116 < 0) goto LAB_0000d4a0;
          if (-1 < DAT_00fff0b3) {
            FUN_00012c5a();
            if ((DAT_00fff0d0 == '\0') || ((DAT_00fff0d1 != '\x02' && (DAT_00fff0d1 != '\x03')))) {
              if (((char)(DAT_00fff0d1 + -4) < '\0') ||
                 (((DAT_00fff0d0 != '\0' || (DAT_00fff0d1 != '\x04')) ||
                  ((char)(*(char *)((int)DAT_00fffc04 + 0x9b) + -2) < '\0')))) {
                pbVar12 = (byte *)((int)DAT_00fffc04 + 0x8d);
                uVar10 = (ushort)*pbVar12 - (ushort)DAT_00fff0b8;
                if (-1 < (int)((uint)uVar10 << 0x10)) {
                  uVar20 = (undefined2)((uint)pbVar12 >> 0x10);
                  local_2 = SUB42(pbVar12,0);
                  FUN_0000d53c();
                  if (DAT_00fff0dc == '\0') {
                    *(char *)CONCAT22(uVar20,local_2) = (char)uVar10;
                    sVar11 = CONCAT11((char)(uVar10 >> 8),DAT_00fff0b7);
                    if ((char)(DAT_00fff0d1 + -4) < '\0') {
                      puVar17 = (undefined2 *)((int)DAT_00fffc04 + 0x77);
                    }
                    else {
                      puVar17 = DAT_00fffc04 + 0x40;
                      if (-1 < (short)(sVar11 + -6)) {
                        puVar17 = (undefined2 *)((int)DAT_00fffc04 + 0x81);
                      }
                    }
                    *(undefined1 *)((int)puVar17 + (int)sVar11) = DAT_00fff0b6;
                  }
                  goto LAB_0000d0d6;
                }
                render_title_text_source();
              }
              else {
                render_title_text_source();
              }
            }
            else {
              render_title_text_source();
            }
          }
        }
      }
      FUN_0000d5dc();
      bVar18 = DAT_00fff0d0 != '\0';
      if (bVar18) {
        _DAT_00fff0d0 = _DAT_00fff0d0 & 0xff;
      }
    } while( true );
  case 4:
    FUN_00012c36();
    FUN_00011900();
    render_title_text_source();
    sVar11 = DAT_00fffc04[0x48];
    if (sVar11 == 0) {
      render_title_text_source();
    }
    else {
      sVar6 = 0xe;
      do {
        if ((CONCAT22((short)((uint)unaff_D3 >> 0x10),sVar11) & 1 << ((ushort)(0xe - sVar6) & 0x1f))
            != 0) {
          render_title_text_source();
        }
        sVar6 = sVar6 + -1;
      } while (sVar6 != -1);
    }
    wait_for_controller_event();
    FUN_00011928();
    return;
  case 5:
    DAT_00fff0db = 0;
    if (((char)(*(char *)((int)DAT_00fffc04 + 0x9b) + -2) < '\0') &&
       (*(char *)((int)DAT_00fffc04 + 0x7f) != '\0')) {
      VDP_CTRL = 0x8134;
      _DAT_00fff0d0 = 0;
      copy_source_word_block();
      DAT_00ffef14 = DAT_00ffef14 | 1;
      FUN_00000ffe();
      decode_and_upload_source_stream();
      render_hud_sprite_stream();
      DAT_00fff10e = &DAT_00fff9fa + (uint)(ushort)DAT_00fffc04[0x50] * 0x11;
      sVar11 = *DAT_00fff10e;
      do {
        FUN_00011190();
        sVar11 = sVar11 + -1;
      } while (sVar11 != -1);
      FUN_00010276();
      VDP_CTRL = 0x8174;
      do {
        while( true ) {
          while( true ) {
            while( true ) {
              FUN_0000fdfe();
              wait_for_vblank();
              latch_controller_events();
              sample_controller_input();
              if (((DAT_00ffdd22._0_1_ & 4) == 0) && ((DAT_00ffdd22._0_1_ & 8) == 0)) break;
              move_selection_column();
              FUN_00010276();
              DAT_00fff0da = 0;
            }
            if (((DAT_00ffdd22._0_1_ & 1) == 0) && ((DAT_00ffdd22._0_1_ & 2) == 0)) break;
            move_selection_row();
            FUN_00010276();
            DAT_00fff0da = 0;
          }
          if ((((DAT_00ffdd22._0_1_ & 0x20) == 0) && ((DAT_00ffdd22._0_1_ & 0x10) == 0)) &&
             ((DAT_00ffdd22._0_1_ & 0x40) == 0)) break;
          FUN_0000ff5a();
          FUN_00010276();
        }
      } while ((DAT_00ffdd22._0_1_ & 0x80) == 0);
      _DAT_00fff0d0 = uVar20;
      return;
    }
    FUN_00011900();
    FUN_00012c36();
    render_title_text_source();
    FUN_000106da();
    FUN_000106da();
    wait_for_controller_event();
    FUN_00011928();
    return;
  case 6:
    FUN_00010a7e();
LAB_0000c2aa:
    FUN_0000c3e6();
    render_title_text_source();
    FUN_0000c3f8();
    DAT_00fff0dc = '\0';
    do {
      move_wrapped_menu_cursor();
      while( true ) {
        wait_for_vblank();
        latch_controller_events();
        sample_controller_input();
        FUN_0000e660();
        if (((DAT_00ffdd22._0_1_ & 1) != 0) || ((DAT_00ffdd22._0_1_ & 2) != 0)) break;
        if ((DAT_00ffdd22._0_1_ & 0x40) != 0) {
          FUN_0000c436();
          if (DAT_00fff0dc < '\0') {
            return;
          }
          goto LAB_0000c2aa;
        }
        if ((DAT_00ffdd22._0_1_ & 0x20) == 0) {
          if ((DAT_00ffdd22._0_1_ & 0x80) != 0) {
            return;
          }
        }
        else {
          FUN_0000391a();
        }
      }
    } while( true );
  case 7:
    break;
  case 8:
    FUN_00012148();
    if ((unaff_D7w == 7) &&
       (((*(char *)(extraout_A0_05 + 0x6d) == ')' || (*(char *)(extraout_A0_05 + 0x6d) == '*')) ||
        (*(char *)(extraout_A0_05 + 0x6d) == '+')))) {
      FUN_00012c42();
      render_title_text_source();
      wait_for_controller_event();
      return;
    }
    DAT_00fff098 = 0;
    FUN_000111d0();
    DAT_00fff114 = '\0';
    FUN_000127b0();
    FUN_00012c42();
    FUN_00012148();
    if (cVar7 == *(char *)(DAT_00fffc04 + 0x2b)) {
      render_title_text_source();
      DAT_00fff098 = 0xffff;
    }
    render_title_text_source();
    do {
      while( true ) {
        wait_for_vblank();
        latch_controller_events();
        sample_controller_input();
        if (((DAT_00ffdd22._0_1_ & 4) == 0) && ((DAT_00ffdd22._0_1_ & 8) == 0)) break;
        FUN_000530de();
        FUN_000127b0();
      }
    } while (((DAT_00ffdd22._0_1_ & 0x40) == 0) && ((DAT_00ffdd22._0_1_ & 0x10) == 0));
    FUN_000530de();
    if (-1 < DAT_00fff114) {
      FUN_00012148();
      FUN_0001275c();
      FUN_00011624();
    }
    return;
  case 9:
    cVar7 = FUN_00012148();
    if ((-1 < (char)(cVar7 + -0x17)) && ((char)(cVar7 + -0x2c) < '\0')) {
      FUN_00012c42();
      render_title_text_source();
      wait_for_controller_event();
      return;
    }
    FUN_00011900();
    puVar17 = DAT_00fffc04;
    FUN_000111d0();
    do {
      DAT_00fff114 = '\0';
      FUN_000127b0();
      puVar4 = DAT_00fffc04;
      DAT_00fff096 = 0;
      puVar14 = puVar17;
      do {
        puVar17 = puVar14 + 0x80;
        if (2 < (short)puVar14[0x9f]) {
          puVar17 = &DAT_00ff0100;
        }
        puVar14 = puVar17;
      } while ((*(byte *)(puVar17 + 8) & 1) == 0);
      if (puVar17 == DAT_00fffc04) goto LAB_000129dc;
      sVar11 = 7;
      do {
        if (*(char *)(puVar14 + 0x33) == '\0') {
          FUN_00012c42();
          render_title_text_source();
          render_title_text_source();
          FUN_000110e8();
          FUN_00012148();
          puVar17 = extraout_A0_06;
          if ((char)puVar4 == *(char *)(extraout_A0_07 + 0x56)) {
            render_title_text_source();
          }
          goto LAB_0001296a;
        }
        puVar14 = (undefined2 *)((int)puVar14 + 1);
        sVar11 = sVar11 + -1;
      } while (sVar11 != -1);
      FUN_00012c42();
      render_title_text_source();
      render_title_text_source();
      DAT_00fff096 = -1;
LAB_0001296a:
      do {
        while( true ) {
          wait_for_vblank();
          latch_controller_events();
          sample_controller_input();
          if (((DAT_00ffdd22._0_1_ & 4) == 0) && ((DAT_00ffdd22._0_1_ & 8) == 0)) break;
          FUN_000530de();
          FUN_000127b0();
        }
      } while (((DAT_00ffdd22._0_1_ & 0x40) == 0) &&
              (((DAT_00ffdd22._0_1_ & 0x20) == 0 && ((DAT_00ffdd22._0_1_ & 0x10) == 0))));
      FUN_000530de();
    } while ((DAT_00fff096 < 0) || (DAT_00fff114 < '\0'));
    FUN_000129fe();
    FUN_00011624();
    FUN_00012c36();
LAB_000129dc:
    FUN_00011928();
    return;
  case 10:
    DAT_00fff112 = 0;
    return;
  default:
    DAT_00fff09f = '\0';
    bVar18 = true;
    DAT_00fff0cd = uVar19;
    cVar7 = FUN_00012148();
    uVar20 = _DAT_00fff0d0;
    if (bVar18) {
      return;
    }
    _DAT_00fff0d0 = CONCAT11(2,DAT_00fff0d1);
    if ((-1 < (char)(cVar7 + -0x10)) && ((char)(cVar7 + -0x13) < '\0')) {
      DAT_00fff09f = cVar7;
    }
    FUN_000111d0();
    _DAT_00fff116 = 0xffff;
    FUN_00011454();
    FUN_00011712();
    do {
      while( true ) {
        wait_for_vblank();
        latch_controller_events();
        sample_controller_input();
        if ((DAT_00ffdd22._0_1_ & 4) == 0) break;
        uVar5 = 0xffff;
LAB_000117f6:
        FUN_000530de(uVar5);
        FUN_000111d0();
        cycle_inventory_action_selector();
        _DAT_00fff116 = 0xffff;
        FUN_00011454();
        FUN_00011712();
      }
      if ((DAT_00ffdd22._0_1_ & 8) != 0) {
        uVar5 = 1;
        goto LAB_000117f6;
      }
      if (((DAT_00ffdd22._0_1_ & 0x40) != 0) || ((DAT_00ffdd22._0_1_ & 0x10) != 0)) {
        FUN_000530de();
        uVar10 = _DAT_00fff0d0 >> 8;
        _DAT_00fff0d0 = uVar20;
        (**(code **)((int)&PTR_caseD_7_000d2a1e + (uint)(ushort)((uVar10 - 2) * 4)))();
        uVar20 = _DAT_00fff0d0;
        goto LAB_000118b6;
      }
      if ((DAT_00ffdd22._0_1_ & 0x80) != 0) {
LAB_000118b6:
        _DAT_00fff0d0 = uVar20;
        FUN_000111d0();
        FUN_0001170c();
        return;
      }
    } while( true );
  }
  cVar8 = FUN_00012148();
  if ('\0' < DAT_00fff09f) {
    FUN_00011900();
    FUN_00012c42();
    DAT_00fff0b9 = '\0';
    puVar16 = &DAT_00fff0ba;
    render_title_text_source();
    if ((DAT_00ff0110 & 1) != 0) {
      DAT_00fff0b9 = DAT_00fff0b9 + '\x01';
      render_title_text_source();
      DAT_00fff0ba = &DAT_00ff0100;
      puVar16 = (undefined4 *)&DAT_00fff0be;
    }
    if ((DAT_00ff0210 & 1) != 0) {
      DAT_00fff0b9 = DAT_00fff0b9 + '\x01';
      render_title_text_source();
      *puVar16 = &DAT_00ff0200;
      puVar16 = puVar16 + 1;
    }
    if ((DAT_00ff0310 & 1) != 0) {
      DAT_00fff0b9 = DAT_00fff0b9 + '\x01';
      render_title_text_source();
      *puVar16 = &DAT_00ff0300;
    }
    render_title_text_source();
    DAT_00fff0dc = '\0';
    do {
      FUN_000530de();
      sVar11 = -0x3962;
      DAT_00fff0fc = 0xff;
      move_wrapped_menu_cursor();
      if (DAT_00fff0dc == DAT_00fff0b9) {
        sVar11 = 3;
        FUN_000011c0();
      }
      else {
        FUN_000110e8();
      }
      while( true ) {
        wait_for_vblank();
        latch_controller_events();
        sample_controller_input();
        if (((DAT_00ffdd22._0_1_ & 1) != 0) || ((DAT_00ffdd22._0_1_ & 2) != 0)) break;
        if (((DAT_00ffdd22._0_1_ & 0x40) != 0) ||
           (((DAT_00ffdd22._0_1_ & 0x10) != 0 || ((DAT_00ffdd22._0_1_ & 0x20) != 0)))) {
          FUN_000530de();
          FUN_00012c1e();
          if (DAT_00fff0dc == DAT_00fff0b9) goto LAB_00012538;
          sVar6 = FUN_00012148();
          bVar18 = (int)((uint)(ushort)(sVar6 - 0x11U) << 0x10) < 0;
          if (bVar18) {
            FUN_0001253e();
            if (!bVar18) {
              pbVar12 = (byte *)(extraout_A0_04 + 0x6e + (int)sVar11);
              bVar1 = *pbVar12;
              bVar2 = bVar1 - 1;
              *pbVar12 = bVar2;
              if (bVar2 == 0 || SBORROW1(bVar1,'\x01') != (int)((uint)bVar2 << 0x18) < 0) {
                FUN_0001275c();
                FUN_00011624();
              }
            }
          }
          else if ((ushort)(sVar6 - 0x11U) == 0) {
            if (*(char *)(extraout_A1 + 0x9c) != '\n') {
              FUN_000037ec();
              cVar7 = *(char *)(extraout_A0_03 + 0x4c);
              iVar13 = extraout_A0_03;
              iVar3 = extraout_A1_01;
              goto joined_r0x00012496;
            }
          }
          else if (*(char *)(extraout_A1 + 0x4c) != '\n') {
            FUN_00003674();
            cVar7 = *(char *)(extraout_A0_02 + 0x9c);
            iVar13 = extraout_A0_02;
            iVar3 = extraout_A1_00;
joined_r0x00012496:
            if (cVar7 != '\0') {
              *(byte *)(iVar13 + 0x10) = *(byte *)(iVar13 + 0x10) & 0xbf;
              *(byte *)(iVar13 + 0x10) = *(byte *)(iVar13 + 0x10) & 0xef;
              *(byte *)(iVar13 + 0x10) = *(byte *)(iVar13 + 0x10) | 1;
            }
            pbVar12 = (byte *)(iVar3 + 0x6e + (int)sVar11);
            bVar1 = *pbVar12;
            bVar2 = bVar1 - 1;
            *pbVar12 = bVar2;
            if (bVar2 == 0 || SBORROW1(bVar1,'\x01') != (int)((uint)bVar2 << 0x18) < 0) {
              FUN_0001275c();
              FUN_00011624();
              FUN_000038fa();
            }
          }
          render_title_text_source();
          wait_for_controller_event();
          goto LAB_00012538;
        }
        if ((DAT_00ffdd22._0_1_ & 0x80) != 0) {
LAB_00012538:
          FUN_00011928();
          return;
        }
      }
    } while( true );
  }
  if (((cVar8 == '\x14') || (cVar8 == '\x16')) || (cVar8 == '\x13')) {
LAB_000121c0:
    FUN_00012c42();
    render_title_text_source();
    wait_for_controller_event();
    return;
  }
  if (-1 < (char)(cVar8 + -0x18)) {
    if ((char)(cVar8 + -0x2c) < '\0') goto LAB_000121c0;
    if (*(char *)(extraout_A0_00 + 0x76) == '\0') {
      *(char *)(extraout_A0_00 + 0x76) = cVar8 + -0x2b;
      *(char *)(extraout_A0_00 + 0x6e + (int)unaff_D7w) = cVar8 + -0x2b;
    }
    else {
      *(undefined1 *)(extraout_A0_00 + 0x76) = 0;
      *(undefined1 *)(extraout_A0_00 + 0x6e + (int)unaff_D7w) = 0;
    }
    goto LAB_0001228c;
  }
  iVar13 = extraout_A0_00;
  if (*(char *)(extraout_A0_00 + 0x56) < '\0') {
LAB_000121fe:
    uVar19 = *(undefined1 *)(extraout_A0_00 + 0x4e);
    *(undefined1 *)(extraout_A0_00 + 0x4e) = 0;
LAB_00012236:
    *(undefined1 *)(iVar13 + 0x9d) = 0;
    *(char *)(iVar13 + 0x56) = cVar7;
    *(char *)(iVar13 + 0x57) = cVar8;
    if (-1 < (char)(*(char *)(iVar13 + 0x57) + -0xd)) {
      *(undefined1 *)(iVar13 + 0x4e) = uVar19;
    }
  }
  else {
    uVar19 = extraout_D1b;
    if ((char)(*(char *)(extraout_A0_00 + 0x57) + -0xd) < '\0') {
      if ((int)((uint)(ushort)(*(byte *)(extraout_A0_00 + 0x57) - 1) << 0x10) < 0)
      goto LAB_000121fe;
      cVar8 = FUN_000125fa();
      iVar13 = extraout_A0_01;
      uVar19 = extraout_D1b_00;
    }
    if (cVar7 != *(char *)(iVar13 + 0x56)) goto LAB_00012236;
    if ((char)(*(char *)(iVar13 + 0x57) + -0xd) < '\0') {
      *(undefined1 *)(iVar13 + 0x4e) = uVar19;
    }
    *(undefined1 *)(iVar13 + 0x56) = 0x7f;
    *(undefined1 *)(iVar13 + 0x57) = 0;
  }
  FUN_000036fc();
LAB_0001228c:
  FUN_00003404();
  FUN_00011624();
  return;
}


