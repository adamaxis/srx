// Ghidra headless export
// Entry: 0x0005825C
// Name: FUN_0005825c
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000C0DA render_title_text_source
//   0x0000C216 build_grouped_decimal_glyphs
//   0x00057A6E FUN_00057a6e
//   0x00057B42 FUN_00057b42
//   0x00057C3A FUN_00057c3a


void FUN_0005825c(void)

{
  char *extraout_A0;
  
  render_title_text_source();
  build_grouped_decimal_glyphs();
  build_grouped_decimal_glyphs();
  build_grouped_decimal_glyphs();
  build_grouped_decimal_glyphs();
  build_grouped_decimal_glyphs();
  build_grouped_decimal_glyphs();
  if (-1 < (char)(DAT_00fffbcb - *extraout_A0)) {
    FUN_00057c3a();
    return;
  }
  if (*(int *)(extraout_A0 + 2) + -0x10000 < 0) {
    FUN_00057a6e();
  }
  else {
    FUN_00057b42();
  }
  build_grouped_decimal_glyphs();
  return;
}


