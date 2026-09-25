// Ghidra headless export
// Entry: 0x000580FE
// Name: convert_intro_routine_0x000580fe
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001232 write_vdp_grid
//   0x0000C0DA render_encoded_title_text
//   0x0000C134 normalize_source_text_line
//   0x0000C168 convert_graphics_routine_0x0000c168
//   0x0000C216 build_grouped_decimal_glyphs
//   0x000575F8 convert_intro_routine_0x000575f8


void convert_intro_routine_0x000580fe(void)

{
  normalize_source_text_line();
  DAT_00ffb740 = 0xff;
  convert_graphics_routine_0x0000c168();
  if ((char)(DAT_00fff0d6 + -9) < '\0') {
    render_encoded_title_text();
    render_encoded_title_text();
    render_encoded_title_text();
    build_grouped_decimal_glyphs();
    convert_intro_routine_0x000575f8();
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


