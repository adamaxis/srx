// Ghidra headless export
// Entry: 0x0000D01C
// Name: run_attributes_and_skills_menu
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000BD2 wait_for_vblank
//   0x00000BF2 latch_controller_events
//   0x00000FFE clear_and_fill_plane_a
//   0x0000102A clear_and_fill_plane_b
//   0x000011C0 write_repeated_vdp_grid
//   0x0000195C sample_controller_input
//   0x0000334C upload_field_asset_sequence
//   0x000039F2 refresh_field_animation_assets
//   0x0000C0DA render_encoded_title_text
//   0x0000C216 build_grouped_decimal_glyphs
//   0x0000D4C8 convert_graphics_routine_0x0000d4c8
//   0x0000D53C convert_graphics_routine_0x0000d53c
//   0x0000D5DC render_cyberware_detail
//   0x0000D82A render_attributes_and_skills
//   0x00010C96 transfer_menu_asset_batch_eight
//   0x000110EC draw_menu_hud_panel_variant_b
//   0x00012C5A write_menu_repeated_grid_with_metadata
//   0x000566E2 refresh_mode_resource_metadata


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void run_attributes_and_skills_menu(void)

{
  char cVar1;
  byte bVar2;
  ushort uVar3;
  short sVar4;
  byte *pbVar5;
  int iVar6;
  bool bVar7;
  
  _DAT_00fff116 = -1;
  VDP_CTRL = 0x9100;
  clear_and_fill_plane_a();
  clear_and_fill_plane_b();
  transfer_menu_asset_batch_eight();
  DAT_00ffd8d4 = 0x2c2;
  DAT_00ffd8f4 = 0x60;
  DAT_00ffef14 = DAT_00ffef14 | 1;
  sVar4 = 3;
  do {
    render_encoded_title_text();
    sVar4 = sVar4 + -1;
  } while (sVar4 != -1);
  sVar4 = 3;
  do {
    render_encoded_title_text();
    sVar4 = sVar4 + -1;
  } while (sVar4 != -1);
  sVar4 = 5;
  do {
    render_encoded_title_text();
    sVar4 = sVar4 + -1;
  } while (sVar4 != -1);
  sVar4 = 5;
  do {
    render_encoded_title_text();
    sVar4 = sVar4 + -1;
  } while (sVar4 != -1);
  _DAT_00fff0d0 = 0;
LAB_0000d0d6:
  write_repeated_vdp_grid();
  write_repeated_vdp_grid();
  refresh_mode_resource_metadata();
  draw_menu_hud_panel_variant_b();
  render_encoded_title_text();
  render_encoded_title_text();
  render_encoded_title_text();
  sVar4 = 3;
  do {
    build_grouped_decimal_glyphs();
    sVar4 = sVar4 + -1;
  } while (sVar4 != -1);
  sVar4 = 1;
  do {
    build_grouped_decimal_glyphs();
    sVar4 = sVar4 + -1;
  } while (sVar4 != -1);
  build_grouped_decimal_glyphs();
  build_grouped_decimal_glyphs();
  build_grouped_decimal_glyphs();
  sVar4 = 5;
  do {
    build_grouped_decimal_glyphs();
    sVar4 = sVar4 + -1;
  } while (sVar4 != -1);
  sVar4 = 5;
  do {
    build_grouped_decimal_glyphs();
    sVar4 = sVar4 + -1;
  } while (sVar4 != -1);
LAB_0000d25e:
  do {
    render_cyberware_detail();
    render_attributes_and_skills();
    while( true ) {
      wait_for_vblank();
      latch_controller_events();
      sample_controller_input();
      if ((DAT_00ffdd22._0_1_ & 4) != 0) break;
      if ((DAT_00ffdd22._0_1_ & 8) != 0) {
        render_cyberware_detail();
        bVar7 = DAT_00fff0d0 == '\0';
        if (bVar7) {
          _DAT_00fff0d0 = CONCAT11(1,DAT_00fff0d1);
        }
        goto LAB_0000d25e;
      }
      if ((DAT_00ffdd22._0_1_ & 1) != 0) {
        render_cyberware_detail();
        bVar2 = DAT_00fff0d1 - 1;
        _DAT_00fff0d0 = CONCAT11(DAT_00fff0d0,bVar2);
        if ((short)((ushort)bVar2 << 8) < 0) {
          _DAT_00fff0d0 = (ushort)DAT_00fff0d0 << 8;
        }
        goto LAB_0000d25e;
      }
      if ((DAT_00ffdd22._0_1_ & 2) != 0) {
        render_cyberware_detail();
        uVar3 = _DAT_00fff0d0;
        cVar1 = DAT_00fff0d1 + '\x01';
        _DAT_00fff0d0 = CONCAT11(DAT_00fff0d0,cVar1);
        if ('\t' < cVar1) {
          _DAT_00fff0d0 = uVar3;
        }
        goto LAB_0000d25e;
      }
      if (((DAT_00ffdd22._0_1_ & 0x40) == 0) && ((DAT_00ffdd22._0_1_ & 0x10) == 0)) {
        if ((DAT_00ffdd22._0_1_ & 0x20) != 0) {
          render_cyberware_detail();
          convert_graphics_routine_0x0000d4c8();
          goto LAB_0000d0d6;
        }
        if ((DAT_00ffdd22._0_1_ & 0x80) != 0) {
LAB_0000d4a0:
          VDP_CTRL = 0x9191;
          clear_and_fill_plane_a();
          clear_and_fill_plane_b();
          _DAT_00fff116 = 0;
          upload_field_asset_sequence();
          refresh_field_animation_assets();
          return;
        }
      }
      else {
        if (_DAT_00fff116 < 0) goto LAB_0000d4a0;
        if (-1 < DAT_00fff0b3) {
          write_menu_repeated_grid_with_metadata();
          if ((DAT_00fff0d0 == '\0') || ((DAT_00fff0d1 != '\x02' && (DAT_00fff0d1 != '\x03')))) {
            if (((char)(DAT_00fff0d1 + -4) < '\0') ||
               (((DAT_00fff0d0 != '\0' || (DAT_00fff0d1 != '\x04')) ||
                ((char)(*(char *)(DAT_00fffc04 + 0x9b) + -2) < '\0')))) {
              pbVar5 = (byte *)(DAT_00fffc04 + 0x8d);
              sVar4 = (ushort)*pbVar5 - (ushort)DAT_00fff0b8;
              if (-1 < sVar4) {
                convert_graphics_routine_0x0000d53c();
                if (DAT_00fff0dc == '\0') {
                  *pbVar5 = (byte)sVar4;
                  sVar4 = CONCAT11((char)((ushort)sVar4 >> 8),DAT_00fff0b7);
                  if ((char)(DAT_00fff0d1 + -4) < '\0') {
                    iVar6 = DAT_00fffc04 + 0x77;
                  }
                  else {
                    iVar6 = DAT_00fffc04 + 0x80;
                    if (-1 < (short)(sVar4 + -6)) {
                      iVar6 = DAT_00fffc04 + 0x81;
                    }
                  }
                  *(undefined1 *)(iVar6 + sVar4) = DAT_00fff0b6;
                }
                goto LAB_0000d0d6;
              }
              render_encoded_title_text();
            }
            else {
              render_encoded_title_text();
            }
          }
          else {
            render_encoded_title_text();
          }
        }
      }
    }
    render_cyberware_detail();
    bVar7 = DAT_00fff0d0 != '\0';
    if (bVar7) {
      _DAT_00fff0d0 = _DAT_00fff0d0 & 0xff;
    }
  } while( true );
}


