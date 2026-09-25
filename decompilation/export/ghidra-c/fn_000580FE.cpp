// Ghidra headless export
// Entry: 0x000580FE
// Name: FUN_000580fe
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001232 write_vdp_grid
//   0x0000C0DA render_encoded_title_text
//   0x0000C134 normalize_source_text_line
//   0x0000C168 FUN_0000c168
//   0x0000C216 build_grouped_decimal_glyphs
//   0x000575F8 FUN_000575f8


void FUN_000580fe(void)

{
  normalize_source_text_line();
  DAT_00ffb740 = 0xff;
  FUN_0000c168();
  if ((char)(DAT_00fff0d6 + -9) < '\0') {
    render_encoded_title_text();
    render_encoded_title_text();
    render_encoded_title_text();
    build_grouped_decimal_glyphs();
    FUN_000575f8();
    build_grouped_decimal_glyphs();
    build_grouped_decimal_glyphs();
    build_grouped_decimal_glyphs();
  }
  else {
    render_encoded_title_text();
  }
  write_vdp_grid();
  return;
}


