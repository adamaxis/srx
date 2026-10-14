// Ghidra headless export
// Entry: 0x0005B056
// Name: advance_title_intro_scroll_fade
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0005AA1E render_title_intro_center_text_batch


void advance_title_intro_scroll_fade(void)

{
  if (DAT_00ffdd08 < -0x1af) {
    if (DAT_00ffdd08 == -0x1b0) {
      render_title_intro_center_text_batch();
    }
    if (DAT_00ffdd0e < 0x78) {
      DAT_00ffdd0e = DAT_00ffdd0e + 1;
      DAT_00ffef14 = DAT_00ffef14 | 8;
    }
  }
  return;
}


