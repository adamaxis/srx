// Ghidra headless export
// Entry: 0x0005A252
// Name: advance_title_intro_persistent_sequence
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001378 FUN_00001378
//   0x000530DE service_audio_if_enabled
//   0x0005A582 begin_title_intro_timed_sprites
//   0x0005B2C2 set_title_intro_entity_color_words
//   0x0005B2E2 step_title_intro_entity_color_words
//   0x0005B32C fill_title_intro_word_block


void advance_title_intro_persistent_sequence(void)

{
  if (DAT_00fff12e < 0xc) {
    if (DAT_00fff130 <= DAT_00ffdf88) {
      if (DAT_00fff12e < 4) {
        DAT_00fff12a = *(undefined4 *)((int)&PTR_PTR_0005a398 + (int)(short)(DAT_00fff12e * 6));
        DAT_00fff130 = DAT_00ffdf88 + *(short *)(&DAT_0005a39c + (short)(DAT_00fff12e * 6));
        DAT_00fff12e = DAT_00fff12e + 1;
      }
      else {
        if (DAT_00fff12e == 4) {
          service_audio_if_enabled();
        }
        if (DAT_00fff12e == 4) {
          DAT_00fff146 = 4;
          set_title_intro_entity_color_words();
          DAT_00fff12e = DAT_00fff12e + 1;
          DAT_00fff130 = DAT_00ffdf88 + 9;
        }
        else if (DAT_00fff130 <= DAT_00ffdf88) {
          step_title_intro_entity_color_words();
          DAT_00fff130 = DAT_00ffdf88 + 9;
          DAT_00fff12e = DAT_00fff12e + 1;
          if (DAT_00fff12e == 9) {
            begin_title_intro_timed_sprites();
          }
          if (0xb < DAT_00fff12e) {
            fill_title_intro_word_block();
          }
        }
      }
    }
    FUN_00001378();
  }
  return;
}


