// Ghidra headless export
// Entry: 0x00011B58
// Name: caseD_2
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000BD2 wait_for_vblank
//   0x00000BF2 latch_controller_events
//   0x00000D94 wait_for_controller_event
//   0x0000195C sample_controller_input
//   0x00003404 upload_field_asset_setup
//   0x000036FC FUN_000036fc
//   0x0000C0DA render_title_text_source
//   0x0000C216 build_grouped_decimal_glyphs
//   0x000111D0 upload_menu_tile_block
//   0x00011454 reset_menu_upload_cursor_and_upload
//   0x00011712 render_title_text_source_variant
//   0x000118C6 cycle_inventory_action_selector
//   0x00011900 prepare_menu_row_grid
//   0x00011928 render_menu_row
//   0x00012062 refresh_menu_text
//   0x0001209A advance_menu_text_cursor
//   0x000120E4 render_menu_number
//   0x000127B0 refresh_menu_panel
//   0x00012C36 prepare_menu_grid_variant_b
//   0x000530DE service_audio_if_enabled


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void switchD_00011768::caseD_2(void)

{
  byte bVar1;
  undefined2 uVar2;
  undefined2 uVar3;
  uint uVar4;
  ushort uVar5;
  short sVar6;
  undefined2 *puVar7;
  int extraout_A0;
  undefined4 *puVar8;
  undefined4 *puVar9;
  int iVar10;
  char cVar11;
  undefined1 uVar12;
  char cVar13;
  
  prepare_menu_grid_variant_b();
  prepare_menu_row_grid();
  render_title_text_source();
  build_grouped_decimal_glyphs();
  uVar2 = _DAT_00fff0d0;
  _DAT_00fff0d0 = CONCAT11(2,DAT_00fff0d1);
  upload_menu_tile_block();
  _DAT_00fff116 = 0xffff;
  reset_menu_upload_cursor_and_upload();
  render_title_text_source_variant();
  do {
    while( true ) {
      wait_for_vblank();
      latch_controller_events();
      sample_controller_input();
      if ((DAT_00ffdd22._0_1_ & 4) == 0) break;
      uVar3 = 0xffff;
LAB_00011be6:
      service_audio_if_enabled(uVar3);
      upload_menu_tile_block();
      cycle_inventory_action_selector();
      _DAT_00fff116 = 0xffff;
      reset_menu_upload_cursor_and_upload();
      render_title_text_source_variant();
    }
    if ((DAT_00ffdd22._0_1_ & 8) != 0) {
      uVar3 = 1;
      goto LAB_00011be6;
    }
  } while ((((DAT_00ffdd22._0_1_ & 0x40) == 0) && ((DAT_00ffdd22._0_1_ & 0x10) == 0)) &&
          ((DAT_00ffdd22._0_1_ & 0x20) == 0));
  cVar11 = '\0';
  uVar12 = 0;
  cVar13 = '\0';
  service_audio_if_enabled();
  refresh_menu_text();
  if ((bool)uVar12 || cVar13 != cVar11) goto LAB_00011c9c;
  _DAT_00fff0d0 = uVar2;
  if (DAT_00fff0d0 == '\x02') {
    prepare_menu_grid_variant_b();
    if ((-1 < (char)(*(char *)(DAT_00fffc04 + 0x56) << 1)) &&
       ((char)(*(char *)(DAT_00fffc04 + 0x57) + -0xd) < '\0')) {
      if (*(char *)(DAT_00fffc04 + 0x4e) != '\0') {
        DAT_00fff114 = '\0';
        upload_menu_tile_block();
        refresh_menu_panel();
        render_title_text_source();
        do {
          while( true ) {
            wait_for_vblank();
            latch_controller_events();
            sample_controller_input();
            if (((DAT_00ffdd22._0_1_ & 4) == 0) && ((DAT_00ffdd22._0_1_ & 8) == 0)) break;
            service_audio_if_enabled();
            refresh_menu_panel();
          }
        } while ((DAT_00ffdd22._0_1_ & 0x40) == 0);
        service_audio_if_enabled();
        if (DAT_00fff114 < '\0') {
          return;
        }
      }
      iVar10 = DAT_00fffc04;
      bVar1 = *(byte *)(DAT_00fffc04 + 0x4f) - 1;
      *(byte *)(DAT_00fffc04 + 0x4f) = bVar1;
      if ((short)((ushort)bVar1 << 8) < 0) {
        if (*(char *)(DAT_00fffc04 + 0x4f) < '\x01') {
          prepare_menu_grid_variant_b();
          render_title_text_source();
          render_title_text_source();
          *(undefined1 *)(extraout_A0 + 0x4f) = 0;
        }
        return;
      }
      if (*(char *)(iVar10 + 0x56) < '\0') {
        *(undefined1 *)(iVar10 + 0x56) = 0x7f;
        *(undefined1 *)(iVar10 + 0x57) = 0;
        upload_field_asset_setup();
      }
      FUN_000036fc();
      prepare_menu_grid_variant_b();
    }
    render_title_text_source();
    goto LAB_00011cb6;
  }
  if (DAT_00fff0d0 == '\x03') {
    prepare_menu_grid_variant_b();
    sVar6 = 2;
    puVar7 = &DAT_00ff0100;
    puVar8 = (undefined4 *)&DAT_00ffdd48;
    do {
      puVar9 = puVar8;
      if (((*(byte *)(puVar7 + 8) & 2) == 0) && ((*(byte *)(puVar7 + 8) & 1) != 0)) {
        puVar9 = puVar8 + 1;
        *puVar8 = puVar7;
      }
      puVar7 = puVar7 + 0x80;
      sVar6 = sVar6 + -1;
      puVar8 = puVar9;
    } while (sVar6 != -1);
    *puVar9 = 0xffffffff;
    if (_DAT_00ffdd48 < 0) goto LAB_00011e14;
    DAT_00ffdd28 = 0;
    render_title_text_source();
    _DAT_00ffdd2a = (uint)*(byte *)(DAT_00fffc04 + 0x4f);
    build_grouped_decimal_glyphs();
    advance_menu_text_cursor();
    uVar2 = _DAT_00fff0d0;
    _DAT_00fff0d0 = CONCAT11(2,DAT_00fff0d1);
    upload_menu_tile_block();
    _DAT_00fff116 = 0xffff;
    reset_menu_upload_cursor_and_upload();
    render_title_text_source_variant();
LAB_00011ea4:
    do {
      while( true ) {
        wait_for_vblank();
        latch_controller_events();
        sample_controller_input();
        if ((DAT_00ffdd22._0_1_ & 4) == 0) break;
        uVar3 = 0xffff;
LAB_00011ec4:
        service_audio_if_enabled(uVar3);
        upload_menu_tile_block();
        cycle_inventory_action_selector();
        _DAT_00fff116 = 0xffff;
        reset_menu_upload_cursor_and_upload();
        render_title_text_source_variant();
      }
      if ((DAT_00ffdd22._0_1_ & 8) != 0) {
        uVar3 = 1;
        goto LAB_00011ec4;
      }
      if ((((DAT_00ffdd22._0_1_ & 0x40) != 0) || ((DAT_00ffdd22._0_1_ & 0x10) != 0)) ||
         ((DAT_00ffdd22._0_1_ & 0x20) != 0)) {
        service_audio_if_enabled();
        if (DAT_00fff0d0 == '\x02') {
          render_menu_number();
        }
        else if (DAT_00fff0d0 == '\x03') {
          render_menu_number();
        }
        else {
          if (DAT_00fff0d0 == '\x04') {
            iVar10 = DAT_00ffdd4c;
            if (DAT_00ffdd28 != 0) {
              iVar10 = _DAT_00ffdd48;
            }
            cVar11 = (char)_DAT_00ffdd2a;
            _DAT_00fff0d0 = uVar2;
            *(char *)(DAT_00fffc04 + 0x4f) = *(char *)(DAT_00fffc04 + 0x4f) - cVar11;
            *(char *)(iVar10 + 0x4f) = cVar11 + *(char *)(iVar10 + 0x4f);
            prepare_menu_grid_variant_b();
            render_title_text_source();
            goto LAB_00011cb6;
          }
          advance_menu_text_cursor();
        }
        goto LAB_00011ea4;
      }
    } while ((DAT_00ffdd22._0_1_ & 0x80) == 0);
  }
  else if (DAT_00fff0d0 == '\x04') {
    prepare_menu_grid_variant_b();
    uVar4 = 0;
    uVar5 = 0;
    sVar6 = 2;
    puVar7 = &DAT_00ff0100;
    do {
      if ((*(byte *)(puVar7 + 8) & 1) != 0) {
        uVar5 = uVar5 + 1;
        uVar4 = (uint)(byte)(*(char *)((int)puVar7 + 0x4f) + (char)uVar4);
      }
      puVar7 = puVar7 + 0x80;
      sVar6 = sVar6 + -1;
    } while (sVar6 != -1);
    if (-1 < (short)(uVar5 - 2)) {
      if ((short)uVar4 != 0) {
        sVar6 = 2;
        puVar7 = &DAT_00ff0100;
        do {
          *(char *)((int)puVar7 + 0x4f) = (char)(uVar4 / uVar5);
          if ((*(byte *)(puVar7 + 8) & 2) != 0) {
            *(char *)((int)puVar7 + 0x4f) =
                 (char)(uVar4 % (uint)uVar5) + *(char *)((int)puVar7 + 0x4f);
          }
          puVar7 = puVar7 + 0x80;
          sVar6 = sVar6 + -1;
        } while (sVar6 != -1);
      }
      prepare_menu_grid_variant_b();
      render_title_text_source();
      goto LAB_00011cb6;
    }
LAB_00011e14:
    render_title_text_source();
    goto LAB_00011cb6;
  }
LAB_00011c9c:
  _DAT_00fff0d0 = uVar2;
  prepare_menu_grid_variant_b();
  render_title_text_source();
LAB_00011cb6:
  upload_menu_tile_block();
  wait_for_controller_event();
  render_menu_row();
  return;
}


