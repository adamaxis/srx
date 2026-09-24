// Ghidra headless export
// Entry: 0x0005A0B2
// Name: advance_title_intro_sequence
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001728 render_hud_sprite_stream
//   0x000530DE service_audio_if_enabled
//   0x0005A582 FUN_0005a582
//   0x0005B2C2 FUN_0005b2c2
//   0x0005B2E2 FUN_0005b2e2
//   0x0005B32C FUN_0005b32c


void advance_title_intro_sequence(void)

{
  if ((DAT_00fff126 < 0xc) && (DAT_00fff128 <= DAT_00ffdf88)) {
    if (DAT_00fff126 < 4) {
      DAT_00fff128 = DAT_00ffdf88 +
                     *(short *)((int)&PTR_DAT_0005a1d6 + (int)(short)(DAT_00fff126 * 6));
      render_hud_sprite_stream();
      DAT_00fff126 = DAT_00fff126 + 1;
    }
    else {
      if (DAT_00fff126 == 4) {
        service_audio_if_enabled();
      }
      if (DAT_00fff126 == 4) {
        FUN_0005b2c2();
        DAT_00fff126 = DAT_00fff126 + 1;
        DAT_00fff128 = DAT_00ffdf88 + 9;
      }
      else if (DAT_00fff128 <= DAT_00ffdf88) {
        FUN_0005b2e2();
        DAT_00fff128 = DAT_00ffdf88 + 9;
        DAT_00fff126 = DAT_00fff126 + 1;
        if (DAT_00fff126 == 9) {
          FUN_0005a582();
        }
        if (0xb < DAT_00fff126) {
          FUN_0005b32c();
        }
      }
    }
  }
  return;
}


