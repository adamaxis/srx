// Ghidra headless export
// Entry: 0x0001745E
// Name: run_field_mode_hud_scene
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000E34 reset_vdp_upload_state
//   0x00000FFE clear_and_fill_plane_a
//   0x0000102A clear_and_fill_plane_b
//   0x00001530 decode_and_upload_source_stream
//   0x00001728 render_hud_sprite_stream
//   0x0000334C upload_field_asset_sequence
//   0x000033BE write_four_by_four_transfer_batch
//   0x00003404 upload_field_asset_setup
//   0x000039F2 refresh_field_animation_assets
//   0x000176BA refresh_field_resource_streams
//   0x000176EA render_menu_footer
//   0x00017708 render_field_hud
//   0x000177A4 initialize_menu_slot_states
//   0x000177CC upload_menu_resource_pair
//   0x000177FC run_menu_hud_tick_loop
//   0x0001789C prepare_menu_hud_sequence
//   0x00017EB4 calculate_menu_delay_offset
//   0x00019856 refresh_field_hud
//   0x0001ADE6 run_world_effect_scene_loop
//   0x0001B5A6 reset_word_stream_area
//   0x0001B628 attach_behavior_descriptor_c
//   0x0001C4EE copy_eleven_words
//   0x000530D2 service_audio_if_enabled_variant
//   0x000530DE service_audio_if_enabled
//   0x000530EA service_audio_command_frame
//   0x00053126 prepare_mode_entry_service
//   0x000531EA copy_audio_state
//   0x00053290 reset_audio_menu_arrays
//   0x000533EE initialize_runner_animation_grid


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void run_field_mode_hud_scene(void)

{
  int extraout_A0;
  
  VDP_CTRL = 0x8134;
  DAT_00ffe189 = DAT_00fff0a0;
  DAT_00fffbca = 10;
  DAT_00fff0a2 = 0;
  DAT_00fff0b1 = '\0';
  DAT_00fff11a = 0xffff;
  if (DAT_00fffbbc != DAT_00fffbbd) {
    reset_audio_menu_arrays();
    DAT_00fffbbd = DAT_00fffbbc;
  }
  DAT_00ffdd48 = 0;
  copy_audio_state();
  calculate_menu_delay_offset();
  DAT_00fff0d2 = 0;
  do {
    VDP_CTRL = 0x8134;
    DAT_00fff0e5 = DAT_00fff0a7;
    DAT_00fff0b7 = 0;
    _DAT_00fff0e6 = 0;
    DAT_00ffdf84 = 0;
    DAT_00fff0a6 = 0;
    DAT_00fff0a9 = 0;
    DAT_00fff0ad = 0;
    DAT_00fff0a3 = 1;
    DAT_00fff0e4 = 0xc;
    DAT_00fff0ac = 0x28;
    initialize_runner_animation_grid();
    if (DAT_00ffdf40 != 0) {
      DAT_00ffe171 = 0xff;
    }
    prepare_mode_entry_service();
    _DAT_00ffe172 = 0xffff;
    _DAT_00ffdf8a = 0xffff;
    DAT_00fff106 = 0xffff;
    DAT_00fff0e1 = 0;
    DAT_00fff0de = 5;
    DAT_00fff0df = 0;
    clear_and_fill_plane_a();
    clear_and_fill_plane_b();
    reset_vdp_upload_state();
    VDP_CTRL = 0x8b03;
    reset_word_stream_area();
    VDP_CTRL = 0x9100;
    render_menu_footer();
    upload_menu_resource_pair();
    decode_and_upload_source_stream();
    render_hud_sprite_stream();
    render_hud_sprite_stream();
    render_field_hud();
    initialize_menu_slot_states();
    refresh_field_hud();
    write_four_by_four_transfer_batch();
    refresh_field_resource_streams();
    copy_eleven_words();
    DAT_00ff158e = 0x120;
    DAT_00ff1590 = 0x12f;
    attach_behavior_descriptor_c();
    *(undefined1 *)(extraout_A0 + 2) = 0x1e;
    *(undefined1 *)(extraout_A0 + 3) = 0x1e;
    prepare_menu_hud_sequence();
    VDP_CTRL = 0x8174;
    service_audio_if_enabled();
    run_menu_hud_tick_loop();
    service_audio_if_enabled_variant();
    service_audio_command_frame();
    render_hud_sprite_stream();
    run_world_effect_scene_loop();
  } while (DAT_00fff0b1 == '\0');
  VDP_CTRL = 0x8134;
  VDP_CTRL = 0x9191;
  upload_field_asset_setup();
  upload_field_asset_sequence();
  refresh_field_animation_assets();
  prepare_mode_entry_service();
  return;
}

