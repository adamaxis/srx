// Ghidra headless export
// Entry: 0x0001989A
// Name: FUN_0001989a
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000C24 wait_for_vblank_frames
//   0x00000D94 wait_for_controller_event
//   0x0000C0DA render_title_text_source
//   0x00018776 advance_hud_fade_service
//   0x00026A00 FUN_00026a00
//   0x000530DE service_audio_if_enabled


void FUN_0001989a(void)

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
        FUN_00026a00();
        render_title_text_source();
        wait_for_vblank_frames();
        wait_for_controller_event();
        return;
      }
    }
  }
  if ((-1 < DAT_00fff0a6) && (render_title_text_source(), DAT_00ffe189 != DAT_00fff0a0)) {
    DAT_00ffe189 = DAT_00fff0a0;
    service_audio_if_enabled();
  }
  return;
}


