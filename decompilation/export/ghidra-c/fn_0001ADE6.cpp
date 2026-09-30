// Ghidra headless export
// Entry: 0x0001ADE6
// Name: run_world_effect_scene_loop
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000BD2 wait_for_vblank
//   0x00000BF2 latch_controller_events
//   0x0000195C sample_controller_input
//   0x000176BA refresh_field_resource_streams
//   0x0001789C prepare_menu_hud_sequence
//   0x00017FDA advance_menu_random_glitch
//   0x0001822C run_matrix_interaction_loop
//   0x000186FA refresh_field_hud_assets
//   0x00018776 advance_hud_fade_service
//   0x00019432 handle_menu_directional_input
//   0x00019788 advance_matrix_program_loading
//   0x0001A370 advance_world_effect_controller
//   0x0001A48A advance_title_animation_cycle
//   0x0001A5A0 advance_world_effect_mode_animation
//   0x0001A7BE append_title_vertical_sprite
//   0x0001A908 initialize_world_effect_mode
//   0x0001AA40 tick_world_effect_animation
//   0x0001B218 advance_title_phase_cycle
//   0x0001B282 advance_matrix_program_transition
//   0x0001B302 advance_title_jitter
//   0x0001B364 advance_title_figure_animation
//   0x0001B45C advance_title_noise_stream
//   0x0001B5A6 reset_word_stream_area
//   0x0001B680 attach_behavior_descriptor_e
//   0x0001B87A dispatch_title_world_phase_service
//   0x0001BA6E advance_title_burst_animation
//   0x0001BB6A advance_title_noise_effect
//   0x0001BCE8 advance_title_profile_motion
//   0x0001BEBA advance_title_profile_transition
//   0x0001BF7A advance_profile_copy_timer
//   0x0001C0C2 advance_alternating_profile
//   0x0001C1F6 advance_audio_menu_countdown
//   0x0001C3FA advance_title_sprite_layers
//   0x0001C61C advance_title_sweep_animation
//   0x0001C754 advance_title_world_interaction_service
//   0x0001C866 advance_title_effect_motion
//   0x0001CA12 advance_world_effect_motion
//   0x0001CC24 advance_persistent_world_effect
//   0x0001CCDC update_persistent_title_effect
//   0x0001CD5A advance_title_secondary_effect_service
//   0x0001D1F4 advance_title_reveal_transfer_service
//   0x0001D280 initialize_world_sequence
//   0x0001D2E6 advance_world_sequence
//   0x000268E6 append_world_sprite_records
//   0x00026958 upload_decoded_source_stream
//   0x00026972 refresh_title_sprite_records
//   0x00026A00 run_display_transition
//   0x000530DE service_audio_if_enabled
//   0x000530EA service_audio_command_frame
//   0x00053126 prepare_mode_entry_service
//   0x00053704 run_runner_selection_screen
//   0x00054662 run_mode_action_loop


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

// Native conversion: shadowrun_ghidra::run_world_effect_scene_loop preserves
// this routine's scene-owned state transitions and callback order without
// carrying VDP registers, ROM pointers, or 68000 control flow into runtime.
//
// Source-specific boundaries corrected in the typed host conversion:
//   * 0x19788 is advance_matrix_program_loading, not a generic world-menu
//     service;
//   * 0x1CCDC and 0x1CC24 are called unconditionally at 0x1B10E/0x1B112;
//   * FFF0B1 is tested with BMI after 0x1A370 but with BNE in the field
//     refresh branch, so the host preserves the signed status byte;
//   * the prologue preserves external FFF0A1/FFE0DE inputs and FFEF14 dirty
//     bits because the ROM does not clear them.
void run_world_effect_scene_loop(void)

{
  short unaff_D7w;
  undefined1 uVar1;
  
  DAT_00ff1596 = 0x120;
  DAT_00ff1598 = 0xd0;
  reset_word_stream_area();
  _DAT_00ffdf8c = 0;
  _DAT_00ffe080 = 0;
  _DAT_00ffe084 = 0;
  _DAT_00ffe0f6 = 0;
  DAT_00ffe0aa = 0;
  _DAT_00ffe0a8 = 0;
  DAT_00ffe058 = 0;
  DAT_00ffe076 = 0;
  _DAT_00ffe0d4 = 0;
  _DAT_00ffe0f4 = 0;
  _DAT_00ffe082 = 0;
  _DAT_00ffe0ac = 0;
  _DAT_00ffe0f2 = 0;
  DAT_00ffe0b6 = 0;
  DAT_00ffe0b8 = 0;
  _DAT_00ffe0c2 = 0;
  _DAT_00ffe0d6 = 0;
  _DAT_00ffe0ee = 0;
  DAT_00fff11e = 0;
  DAT_00ff1592 = 0;
  _DAT_00ffe05e = 0;
  _DAT_00ffe06e = 0;
  DAT_00ff15a4 = 0;
  DAT_00ffe189 = DAT_00fff0a0;
  DAT_00ffe16d = '\0';
  upload_decoded_source_stream();
  initialize_world_effect_mode();
  attach_behavior_descriptor_e();
  do {
    wait_for_vblank();
    latch_controller_events();
    sample_controller_input();
    advance_menu_random_glitch();
    uVar1 = 0;
    if (_DAT_00ffdf8c == 0) {
      uVar1 = 0;
      if (_DAT_00ffe080 != 0) {
        initialize_world_sequence();
        goto LAB_0001aedc;
      }
      if (_DAT_00ffe084 == 0) {
        if ((DAT_00fffbca == '\0') || (*(char *)(DAT_00fffc04 + 0x4c) == '\0')) {
          _DAT_00ffe084 = 1;
          DAT_00ffe086 = 0;
        }
      }
      else {
        advance_title_jitter();
        if (_DAT_00ffe084 == 0) {
          advance_hud_fade_service();
          run_display_transition();
          return;
        }
      }
      advance_matrix_program_loading();
      advance_title_world_interaction_service();
      handle_menu_directional_input();
      advance_world_effect_controller();
      if (DAT_00fff0b1 < '\0') {
        return;
      }
      if ((DAT_00ffe16d != '\0') && (DAT_00ffe16d = DAT_00ffe16d + -1, DAT_00ffe16d == '\0')) {
        service_audio_if_enabled();
      }
      if (DAT_00ffe0aa == 0) goto LAB_0001afc0;
      advance_alternating_profile();
    }
    else {
LAB_0001aedc:
      advance_world_sequence();
      if ((bool)uVar1) {
        _DAT_00ffdf8c = 0;
        prepare_mode_entry_service();
        service_audio_command_frame();
        if (DAT_00fff0a9 == '\0') {
          run_mode_action_loop();
        }
        else {
          run_runner_selection_screen();
        }
        prepare_mode_entry_service();
        return;
      }
LAB_0001afc0:
        advance_profile_copy_timer();
    }
    DAT_00ffdd02 = 1;
    if (_DAT_00ffdf8c == 0) {
      if (DAT_00ffe076 == 4) {
        append_title_vertical_sprite();
      }
      if (_DAT_00ffe0f6 != 0) {
        advance_title_phase_cycle();
      }
      refresh_title_sprite_records();
      advance_title_sprite_layers();
      if ((DAT_00ff1592 & 1) != 0) {
        advance_title_sweep_animation();
      }
      if ((DAT_00ff1592 & 2) != 0) {
        advance_audio_menu_countdown();
      }
      if ((DAT_00ff1592 & 4) != 0) {
        advance_title_noise_stream();
      }
      if ((DAT_00ff1592 & 0x10) != 0) {
        advance_title_reveal_transfer_service();
      }
      if ((DAT_00ff1592 & 0x40) != 0) {
        advance_title_secondary_effect_service();
      }
      if ((DAT_00ff1592 & 0x20) != 0) {
        dispatch_title_world_phase_service();
      }
      if (_DAT_00ffe0ac != 0) {
        advance_title_burst_animation();
      }
      if ((DAT_00fff0a1 & 1) != 0) {
        advance_title_noise_effect();
      }
      if ((DAT_00ff1592 & 0x80) != 0) {
        advance_title_profile_transition();
      }
      if (DAT_00ffe058 != 0) {
        advance_title_profile_motion();
      }
      if (_DAT_00ffe05e != 0) {
      advance_title_effect_motion();
      }
      if (_DAT_00ffe06e != 0) {
        advance_world_effect_motion();
      }
      if (DAT_00ffe076 != 0) {
        advance_world_effect_mode_animation();
      }
      if (_DAT_00ffe0d4 != 0) {
        advance_title_animation_cycle();
      }
      if (_DAT_00ffe0de != 0) {
      advance_matrix_program_transition();
      }
      update_persistent_title_effect();
      advance_persistent_world_effect();
    }
    else {
      advance_title_figure_animation();
    }
    if (_DAT_00ffe080 == 0) {
  tick_world_effect_animation();
    }
    append_world_sprite_records();
    _DAT_00ffd980 = 1;
    if ((_DAT_00ffdf8c == 0) && ((DAT_00ffdd22._0_1_ & 0x80) != 0)) {
      prepare_mode_entry_service();
      service_audio_command_frame();
      reset_word_stream_area();
      run_matrix_interaction_loop();
      if (DAT_00fff0b1 != '\0') {
        return;
      }
      refresh_field_resource_streams();
      if (unaff_D7w == 0) {
        return;
      }
      refresh_field_hud_assets();
      prepare_menu_hud_sequence();
      VDP_CTRL = 0x8174;
      prepare_mode_entry_service();
      service_audio_command_frame();
    }
    DAT_00ffef14 = DAT_00ffef14 | 2;
  } while( true );
}

