// Ghidra headless export
// Entry: 0x00017BC0
// Name: refresh_menu_number_or_grid
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000C0DA render_title_text_source
//   0x0000C216 build_grouped_decimal_glyphs
//   0x00012C5A write_menu_repeated_grid_with_metadata
//   0x00017C52 render_menu_text_followup


void refresh_menu_number_or_grid(void)

{
  if (-1 < (char)(&DAT_00fffbd8)[(short)(ushort)DAT_00fff0d4]) {
    render_title_text_source();
    build_grouped_decimal_glyphs();
    render_menu_text_followup();
    return;
  }
  write_menu_repeated_grid_with_metadata();
  return;
}


