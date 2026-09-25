// Ghidra headless export
// Entry: 0x00010D30
// Name: convert_menu_routine_0x00010d30
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000BD2 wait_for_vblank
//   0x00000BF2 latch_controller_events
//   0x00000E34 reset_vdp_upload_state
//   0x0000195C sample_controller_input
//   0x0000391A handle_mode_action_event
//   0x0000C0C2 load_title_font
//   0x00010F50 prepare_command_overview
//   0x000110BC draw_menu_hud_panel_variant_a
//   0x0001111A render_title_text_source_wrapper
//   0x000111D0 upload_menu_tile_block
//   0x000111E8 convert_menu_routine_0x000111e8
//   0x00011454 reset_menu_upload_cursor_and_upload
//   0x000114A0 convert_menu_routine_0x000114a0
//   0x00011506 convert_menu_routine_0x00011506
//   0x00011592 convert_menu_routine_0x00011592
//   0x00011624 convert_menu_routine_0x00011624
//   0x0001170C render_title_text_source_leaf
//   0x00011740 run_inventory_actions
//   0x00011928 render_menu_row
//   0x000530DE service_audio_if_enabled
//   0x0005670E refresh_mode_palette_data


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void convert_menu_routine_0x00010d30(void)

{
  undefined2 uVar1;
  
  uVar1 = _DAT_00fff0d0;
code_r0x00010d30:
  _DAT_00fff0d0 = uVar1;
  if (((*(byte *)(DAT_00fffc04 + 0x10) & 0x40) != 0) ||
     ((*(byte *)(DAT_00fffc04 + 0x10) & 0x10) != 0)) {
    handle_mode_action_event();
  }
  DAT_00fff09d = 0xff;
  DAT_00fff0b1 = '\0';
  _DAT_00fff112 = 0;
  VDP_CTRL = 0x8134;
  DAT_00fff0e7 = 0;
  DAT_00fff100 = 0;
  prepare_command_overview();
  refresh_mode_palette_data();
  load_title_font();
  VDP_CTRL = 0x8174;
  uVar1 = _DAT_00fff0d0;
LAB_00010d90:
  _DAT_00fff0d0 = uVar1;
  DAT_00fff0e0 = 0;
  draw_menu_hud_panel_variant_a();
  render_title_text_source_wrapper();
  render_menu_row();
  convert_menu_routine_0x00011624();
  upload_menu_tile_block();
  reset_menu_upload_cursor_and_upload();
  _DAT_00fff112 = CONCAT11(0xff,DAT_00fff112_1);
LAB_00010e2e:
  do {
    service_audio_if_enabled();
    render_title_text_source_leaf();
    convert_menu_routine_0x000111e8();
    while( true ) {
      wait_for_vblank();
      latch_controller_events();
      sample_controller_input();
      if ((DAT_00ffdd22._0_1_ & 4) != 0) break;
      if ((DAT_00ffdd22._0_1_ & 1) != 0) {
LAB_00010e20:
        convert_menu_routine_0x00011506();
        if (DAT_00fff0b1 != '\0') goto LAB_00010f22;
        goto LAB_00010e2e;
      }
      if ((DAT_00ffdd22._0_1_ & 8) != 0) break;
      if ((DAT_00ffdd22._0_1_ & 2) != 0) goto LAB_00010e20;
      if (((DAT_00ffdd22._0_1_ & 0x40) == 0) && ((DAT_00ffdd22._0_1_ & 0x10) == 0)) {
        if ((DAT_00ffdd22._0_1_ & 0x20) != 0) {
          handle_mode_action_event();
          reset_vdp_upload_state();
          _DAT_00fff0d0 = 0x600;
          convert_menu_routine_0x000114a0();
          DAT_00fff115 = 0xff;
          convert_menu_routine_0x00011506();
          uVar1 = _DAT_00fff0d0;
          goto LAB_00010d90;
        }
        if ((DAT_00ffdd22._0_1_ & 0x80) != 0) {
LAB_00010f22:
          if (((*(byte *)(DAT_00fffc04 + 0x10) & 0x40) != 0) ||
             ((*(byte *)(DAT_00fffc04 + 0x10) & 0x10) != 0)) {
            handle_mode_action_event();
          }
          DAT_00fff09d = 0;
          DAT_00fff0b1 = 0;
          return;
        }
      }
      else {
        service_audio_if_enabled();
        uVar1 = _DAT_00fff0d0;
        run_inventory_actions();
        DAT_00fff0d0 = SUB21(uVar1,1);
        _DAT_00fff0d0 = uVar1;
        if ('\x01' < DAT_00fff0d0) {
          if (DAT_00fff0d0 == '\x03') goto code_r0x00010d30;
          if (DAT_00fff0d0 == '\x06') {
            if (DAT_00fff0cd != '\0') {
              DAT_00fff0cd = '\0';
              goto LAB_00010f22;
            }
          }
          else if (('\x01' < *(char *)(DAT_00fffc04 + 0x9b)) || (DAT_00fff0d0 != '\x05'))
          goto LAB_00010d90;
          goto code_r0x00010d30;
        }
        convert_menu_routine_0x00011592();
      }
    }
    convert_menu_routine_0x000114a0();
  } while( true );
}


