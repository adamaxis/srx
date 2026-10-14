// Ghidra headless export
// Entry: 0x0001989A
// Name: update_hud_mode_transition
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000C24 wait_for_vblank_frames
//   0x00000D94 wait_for_controller_event
//   0x0000C0DA render_encoded_title_text
//   0x00018776 advance_hud_fade_service
//   0x00026A00 run_display_transition
//   0x000530DE service_audio_if_enabled


void update_hud_mode_transition(void)

{
  char in_D1b;
  
  DAT_00fff0a0 = in_D1b + DAT_00fff0a0;
  if ((DAT_00fff0a0 != '\0') && (DAT_00fff0a0 != '\x01')) {
    if (DAT_00fff0a0 < '\x02') {
      DAT_00fff0a0 = -1;
    }
    else {
      DAT_00fff0a0 = '\x01';
      if (DAT_00fff0a6 != '\0') {
        advance_hud_fade_service();
        run_display_transition();
        render_encoded_title_text();
        wait_for_vblank_frames();
        wait_for_controller_event();
        return;
      }
    }
  }
  if ((-1 < DAT_00fff0a6) && (render_encoded_title_text(), DAT_00ffe189 != DAT_00fff0a0)) {
    DAT_00ffe189 = DAT_00fff0a0;
    service_audio_if_enabled();
  }
  return;
}


