// Ghidra headless export
// Entry: 0x00001C1E
// Name: advance_title_text_state
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x000010D6 step_palette_toward_target
//   0x00001DF6 draw_title_copyright
//   0x000530EA FUN_000530ea


void advance_title_text_state(void)

{
  if (DAT_00ffdd5a == 1) {
    DAT_00ffdd5c = DAT_00ffdd5c - 1;
    if ((short)DAT_00ffdd5c < 0) {
      DAT_00ffdd5a = 2;
      FUN_000530ea();
      draw_title_copyright();
    }
    else if ((DAT_00ffdd5c & 0x1f) == 0) {
      step_palette_toward_target();
      DAT_00ffef14 = DAT_00ffef14 | 1;
      return;
    }
  }
  return;
}


