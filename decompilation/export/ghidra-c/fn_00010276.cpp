// Ghidra headless export
// Entry: 0x00010276
// Name: run_menu_text_display_sequence
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000C0DA render_encoded_title_text
//   0x00010172 refresh_magic_force_display
//   0x00010260 prepare_menu_grid_then_service
//   0x00011190 transfer_menu_glyph_triple
//
// Source behavior: after the grid/force refresh, choose tile mask 0x2000 for
// spell codes below six or 0x6000 otherwise, transfer the selected record's
// glyph triple, render its name and description, then branch into the shared
// 0x102EC tail for Success/Force labels, posture, availability, and force
// value construction.


void run_menu_text_display_sequence(void)

{
  uint unaff_D7;
  
  prepare_menu_grid_then_service();
  refresh_magic_force_display();
  transfer_menu_glyph_triple();
  render_encoded_title_text();
  render_encoded_title_text();
  render_encoded_title_text();
  render_encoded_title_text();
  adjust_runner_posture((unaff_D7 & 0xffff) * 0x30);
  render_magic_spell_availability();
  build_grouped_decimal_glyphs();
  return;
}


