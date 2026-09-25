// Ghidra headless export
// Entry: 0x00058AA4
// Name: render_field_mode_glyph_row
// Decompile status: source-shaped annotation
//
// The native conversion is ghidra_conversion::render_field_mode_glyph_row().
// It keeps the eight glyph-triple submissions and the selection-4 text pass.
// Calls recovered from direct call instructions:
//   0x0000C0DA render_encoded_title_text
//   0x00011190 transfer_menu_glyph_triple


void render_field_mode_glyph_row(void)

{
  short sVar1;
  
  sVar1 = 7;
  do {
    transfer_menu_glyph_triple();
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  if (DAT_00fff0b8 == '\x04') {
    render_encoded_title_text();
  }
  return;
}

