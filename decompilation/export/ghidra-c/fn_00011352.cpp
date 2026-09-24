// Ghidra headless export
// Entry: 0x00011352
// Name: FUN_00011352
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001232 FUN_00001232
//   0x0000C0DA render_title_text_source
//   0x00055E92 FUN_00055e92
//   0x00055EB4 FUN_00055eb4


void FUN_00011352(void)

{
  char in_D0b;
  short unaff_D7w;
  int in_A0;
  int extraout_A0;
  
  if (DAT_00fff0b1 < '\0') {
    DAT_00ffef96 = *(undefined1 *)(in_A0 + 0x76);
    *(char *)(in_A0 + 0x76) = in_D0b + -0x2b;
    DAT_00ffef97 = *(undefined1 *)(in_A0 + 0x6e + (int)unaff_D7w);
    *(char *)(in_A0 + 0x6e + (int)unaff_D7w) = in_D0b + -0x2b;
  }
  else if (*(char *)(in_A0 + 0x76) == '\0') {
    render_title_text_source();
    return;
  }
  render_title_text_source();
  render_title_text_source();
  FUN_00055eb4();
  FUN_00001232();
  FUN_00055e92();
  FUN_00001232();
  if (DAT_00fff0b1 < '\0') {
    DAT_00fff0b1 = '\0';
    *(undefined1 *)(extraout_A0 + 0x76) = DAT_00ffef96;
    *(undefined1 *)(extraout_A0 + 0x6e + (int)unaff_D7w) = DAT_00ffef97;
  }
  return;
}


