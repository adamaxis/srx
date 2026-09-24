// Ghidra headless export
// Entry: 0x000580FE
// Name: FUN_000580fe
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001232 FUN_00001232
//   0x0000C0DA render_title_text_source
//   0x0000C134 normalize_source_text_line
//   0x0000C168 FUN_0000c168
//   0x0000C216 FUN_0000c216
//   0x000575F8 FUN_000575f8


void FUN_000580fe(void)

{
  normalize_source_text_line();
  DAT_00ffb740 = 0xff;
  FUN_0000c168();
  if ((char)(DAT_00fff0d6 + -9) < '\0') {
    render_title_text_source();
    render_title_text_source();
    render_title_text_source();
    FUN_0000c216();
    FUN_000575f8();
    FUN_0000c216();
    FUN_0000c216();
    FUN_0000c216();
  }
  else {
    render_title_text_source();
  }
  FUN_00001232();
  return;
}


