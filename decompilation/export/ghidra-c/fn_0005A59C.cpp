// Ghidra headless export
// Entry: 0x0005A59C
// Name: advance_title_intro_timed_sprites
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001378 FUN_00001378


void advance_title_intro_timed_sprites(void)

{
  if (DAT_00fff150 <= DAT_00ffdf88) {
    if (5 < DAT_00fff14e) {
      return;
    }
    DAT_00fff14a = *(undefined4 *)(&DAT_0005a602 + (short)(DAT_00fff14e * 4));
    DAT_00fff150 = DAT_00ffdf88 + 8;
    DAT_00fff14e = DAT_00fff14e + 1;
  }
  FUN_00001378();
  return;
}


