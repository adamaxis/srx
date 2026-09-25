// Ghidra headless export
// Entry: 0x0001ADE6
// Name: convert_world_routine_0x0001ade6
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000BD2 wait_for_vblank
//   0x00000BF2 latch_controller_events
//   0x0000195C sample_controller_input
//   0x000176BA refresh_field_resource_streams
//   0x0001789C prepare_menu_hud_sequence
//   0x00017FDA advance_menu_random_glitch
//   0x0001822C convert_world_routine_0x0001822c
//   0x000186FA refresh_field_hud_assets
//   0x00018776 advance_hud_fade_service
//   0x00019432 handle_menu_directional_input
//   0x00019788 convert_world_routine_0x00019788
//   0x0001A370 convert_world_routine_0x0001a370
//   0x0001A48A advance_title_animation_cycle
//   0x0001A5A0 convert_world_routine_0x0001a5a0
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
//   0x0001B87A dispatch_title_phase_handler
//   0x0001BA6E convert_world_routine_0x0001ba6e
//   0x0001BB6A convert_world_routine_0x0001bb6a
//   0x0001BCE8 convert_world_routine_0x0001bce8
//   0x0001BEBA convert_world_routine_0x0001beba
//   0x0001BF7A convert_world_routine_0x0001bf7a
//   0x0001C0C2 advance_alternating_profile
//   0x0001C1F6 advance_audio_menu_countdown
//   0x0001C3FA convert_world_routine_0x0001c3fa
//   0x0001C61C convert_world_routine_0x0001c61c
//   0x0001C754 convert_world_routine_0x0001c754
//   0x0001C866 convert_world_routine_0x0001c866
//   0x0001CA12 convert_world_routine_0x0001ca12
//   0x0001CC24 convert_world_routine_0x0001cc24
//   0x0001CCDC update_persistent_title_effect
//   0x0001CD5A convert_world_routine_0x0001cd5a
//   0x0001D1F4 convert_world_routine_0x0001d1f4
//   0x0001D280 convert_world_routine_0x0001d280
//   0x0001D2E6 advance_world_sequence
//   0x000268E6 append_world_sprite_records
//   0x00026958 upload_decoded_source_stream
//   0x00026972 convert_world_routine_0x00026972
//   0x00026A00 convert_world_routine_0x00026a00
//   0x000530DE service_audio_if_enabled
//   0x000530EA service_audio_command_frame
//   0x00053126 prepare_mode_entry_service
//   0x00053704 convert_audio_routine_0x00053704
//   0x00054662 convert_graphics_routine_0x00054662


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void convert_world_routine_0x0001ade6(void)

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
        convert_world_routine_0x0001d280();
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
          convert_world_routine_0x00026a00();
          return;
        }
      }
      convert_world_routine_0x00019788();
      convert_world_routine_0x0001c754();
      handle_menu_directional_input();
      convert_world_routine_0x0001a370();
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
          convert_graphics_routine_0x00054662();
        }
        else {
          convert_audio_routine_0x00053704();
        }
        prepare_mode_entry_service();
        return;
      }
LAB_0001afc0:
      convert_world_routine_0x0001bf7a();
    }
    DAT_00ffdd02 = 1;
    if (_DAT_00ffdf8c == 0) {
      if (DAT_00ffe076 == 4) {
        append_title_vertical_sprite();
      }
      if (_DAT_00ffe0f6 != 0) {
        advance_title_phase_cycle();
      }
      convert_world_routine_0x00026972();
      convert_world_routine_0x0001c3fa();
      if ((DAT_00ff1592 & 1) != 0) {
        convert_world_routine_0x0001c61c();
      }
      if ((DAT_00ff1592 & 2) != 0) {
        advance_audio_menu_countdown();
      }
      if ((DAT_00ff1592 & 4) != 0) {
        advance_title_noise_stream();
      }
      if ((DAT_00ff1592 & 0x10) != 0) {
        convert_world_routine_0x0001d1f4();
      }
      if ((DAT_00ff1592 & 0x40) != 0) {
        convert_world_routine_0x0001cd5a();
      }
      if ((DAT_00ff1592 & 0x20) != 0) {
        dispatch_title_phase_handler();
      }
      if (_DAT_00ffe0ac != 0) {
        convert_world_routine_0x0001ba6e();
      }
      if ((DAT_00fff0a1 & 1) != 0) {
        convert_world_routine_0x0001bb6a();
      }
      if ((DAT_00ff1592 & 0x80) != 0) {
        convert_world_routine_0x0001beba();
      }
      if (DAT_00ffe058 != 0) {
        convert_world_routine_0x0001bce8();
      }
      if (_DAT_00ffe05e != 0) {
        convert_world_routine_0x0001c866();
      }
      if (_DAT_00ffe06e != 0) {
        convert_world_routine_0x0001ca12();
      }
      if (DAT_00ffe076 != 0) {
        convert_world_routine_0x0001a5a0();
      }
      if (_DAT_00ffe0d4 != 0) {
        advance_title_animation_cycle();
      }
      if (_DAT_00ffe0de != 0) {
      advance_matrix_program_transition();
      }
      update_persistent_title_effect();
      convert_world_routine_0x0001cc24();
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
      convert_world_routine_0x0001822c();
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

