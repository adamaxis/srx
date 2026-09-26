// Ghidra headless export
// Entry: 0x000580FE
// Name: render_field_action_description
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001232 write_vdp_grid
//   0x0000C0DA render_encoded_title_text
//   0x0000C134 normalize_source_text_line
//   0x0000C168 render_wrapped_encoded_text
//   0x0000C216 build_grouped_decimal_glyphs
//   0x000575F8 prepare_field_action_resource_selection


void render_field_action_description(void)

{
  normalize_source_text_line();
  DAT_00ffb740 = 0xff;
  render_wrapped_encoded_text();
  if ((char)(DAT_00fff0d6 + -9) < '\0') {
    render_encoded_title_text();
    render_encoded_title_text();
    render_encoded_title_text();
    build_grouped_decimal_glyphs();
    prepare_field_action_resource_selection();
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


