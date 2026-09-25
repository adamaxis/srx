// Ghidra headless export
// Entry: 0x00010276
// Name: run_menu_text_display_sequence
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000C0DA render_encoded_title_text
//   0x00010172 FUN_00010172
//   0x00010260 prepare_menu_grid_then_service
//   0x00011190 transfer_menu_glyph_triple


void run_menu_text_display_sequence(void)

{
  uint unaff_D7;
  
  prepare_menu_grid_then_service();
  FUN_00010172();
  transfer_menu_glyph_triple();
  render_encoded_title_text();
  render_encoded_title_text();
  render_encoded_title_text();
  render_encoded_title_text();
  FUN_000107ce((unaff_D7 & 0xffff) * 0x30);
  FUN_0001088c();
  build_grouped_decimal_glyphs();
  return;
}


