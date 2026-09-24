// Ghidra headless export
// Entry: 0x0005B100
// Name: advance_title_intro_pulse
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001378 FUN_00001378


void advance_title_intro_pulse(void)

{
  if (-0x1b9 < DAT_00ffdd08) {
    if (DAT_00fff192 <= DAT_00ffdf88) {
      DAT_00fff18c = *(undefined4 *)((int)&PTR_DAT_0005b180 + (int)(short)(DAT_00fff190 * 4));
      DAT_00fff192 = DAT_00ffdf88 + 6;
      DAT_00fff190 = DAT_00fff190 + 1;
      if (5 < DAT_00fff190) {
        DAT_00fff190 = 0;
      }
    }
    FUN_00001378();
  }
  return;
}


