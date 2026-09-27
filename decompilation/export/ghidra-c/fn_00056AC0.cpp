// Ghidra headless export
// Entry: 0x00056AC0
// Name: run_field_mode_service
// Decompile status: source-shaped annotation
//
// The native conversion is ghidra_conversion::run_field_mode_service. It
// preserves the field-service setup, panel selection, controller precedence,
// restart/return paths, and the explicit 0x56DC4 action-handler boundary.
// Genesis VDP writes, ROM pointers, and controller globals are represented by
// typed host callbacks in the conversion module.
// Calls recovered from direct call instructions:
//   0x00000BD2 wait_for_vblank
//   0x00000BF2 latch_controller_events
//   0x00000D94 wait_for_controller_event
//   0x00000E34 reset_vdp_upload_state
//   0x00000F5C write_vdp_word_block_with_source_wrap
//   0x00001530 decode_and_upload_source_stream
//   0x0000195C sample_controller_input
//   0x0000334C upload_field_asset_sequence
//   0x00003404 upload_field_asset_setup
//   0x000039F2 refresh_field_animation_assets
//   0x0000C0C2 load_title_font
//   0x0000C0DA render_encoded_title_text
//   0x0000C406 prepare_numeric_menu_block
//   0x0000D4C8 advance_field_active_runner_selection
//   0x00010BC4 transfer_menu_asset_batch_eight_after_copy
//   0x00011048 upload_resource_then_write_two_blocks
//   0x000110EC draw_active_runner_hud_panel
//   0x000530DE service_audio_if_enabled
//   0x000566E2 refresh_mode_resource_metadata
//   0x000566EC copy_field_mode_palette_bank
//   0x000566F6 refresh_mode_resource_state
//   0x0005670E refresh_mode_palette_data
//   0x00056DC4 run_field_action_selection
//   0x000587DA render_field_mode_selection_presentation
//   0x00058B5E refresh_field_mode_panel_grid
//   0x00058BB6 render_field_mode_selection
//   0x00058C54 advance_field_mode_primary_cursor
//   0x00058CDE advance_field_mode_panel_cursor
//   0x00058D32 refresh_field_mode_selection_sprites


void run_field_mode_service(void)

{
  DAT_00fff0b1 = 0;
  VDP_CTRL = 0x9100;
  transfer_menu_asset_batch_eight_after_copy();
  upload_resource_then_write_two_blocks();
  refresh_mode_palette_data();
  if (DAT_00fff0b7 == '\x01') {
    write_vdp_word_block_with_source_wrap();
  }
  if (DAT_00fff0b7 == '\x02') {
    refresh_mode_resource_metadata();
    refresh_mode_resource_state();
    decode_and_upload_source_stream();
  }
  load_title_font();
  render_encoded_title_text();
LAB_00056b66:
  if (DAT_00fff0b7 == '\0') {
    DAT_00fff10e = &DAT_00fff9fa + (uint)*(ushort *)(DAT_00fffc04 + 0xa0) * 0x11;
  }
  reset_vdp_upload_state();
  DAT_00fff112 = '\0';
  DAT_00fff0d0 = '\0';
  DAT_00fff0b8 = '\0';
  DAT_00fff0c2._0_1_ = -1;
  copy_field_mode_palette_bank();
  draw_active_runner_hud_panel();
  do {
    DAT_00fff0d1 = '\0';
    prepare_numeric_menu_block();
    if ((DAT_00fff0b7 == '\x02') && ((DAT_00ff018f & 1) == 0)) {
      render_encoded_title_text();
      wait_for_controller_event();
LAB_00056da8:
      upload_field_asset_sequence();
      refresh_field_animation_assets();
      upload_field_asset_setup();
      VDP_CTRL = 0x9191;
      return;
    }
    if (DAT_00fff0b7 == '\x01') {
      DAT_00fff0d1 = '\x01';
      refresh_field_mode_selection_sprites();
    }
    else {
      refresh_field_mode_panel_grid();
      render_field_mode_selection();
    }
    render_field_mode_selection_presentation();
LAB_00056c54:
    while( true ) {
      wait_for_vblank();
      latch_controller_events();
      sample_controller_input();
      if ((DAT_00ffdd22._0_1_ & 4) == 0) break;
      service_audio_if_enabled();
LAB_00056cb6:
      advance_field_mode_primary_cursor();
    }
    if ((DAT_00ffdd22._0_1_ & 1) != 0) {
      service_audio_if_enabled();
LAB_00056cd4:
      if (DAT_00fff0b7 != '\x01') {
        advance_field_mode_panel_cursor();
      }
      goto LAB_00056c54;
    }
    if ((DAT_00ffdd22._0_1_ & 8) != 0) {
      service_audio_if_enabled();
      goto LAB_00056cb6;
    }
    if ((DAT_00ffdd22._0_1_ & 2) != 0) {
      service_audio_if_enabled();
      goto LAB_00056cd4;
    }
    if (((DAT_00ffdd22._0_1_ & 0x40) == 0) && ((DAT_00ffdd22._0_1_ & 0x10) == 0)) {
      if ((DAT_00ffdd22._0_1_ & 0x20) != 0) {
        service_audio_if_enabled();
        advance_field_active_runner_selection();
        goto LAB_00056b66;
      }
      if ((DAT_00ffdd22._0_1_ & 0x80) != 0) goto LAB_00056da8;
      goto LAB_00056c54;
    }
    service_audio_if_enabled();
    if (DAT_00fff0d1 == '\0') {
      if (((DAT_00fff0d0 == '\x03') && (DAT_00fff0b7 < '\0')) && (DAT_00fff0c2._0_1_ < '\0')) {
        render_encoded_title_text();
      }
      else {
        render_field_mode_selection_presentation();
      }
      goto LAB_00056c54;
    }
    run_field_action_selection();
    if (DAT_00fff112 < '\0') goto LAB_00056c54;
    reset_vdp_upload_state();
    DAT_00fff0d0 = DAT_00fff0b8;
  } while( true );
}

