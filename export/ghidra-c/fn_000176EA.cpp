// Ghidra headless export
// Entry: 0x000176EA
// Name: render_menu_footer
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001358 copy_source_word_block
//   0x00017F8E draw_hud_footer
//   0x00019DF2 run_menu_metadata_service


void render_menu_footer(void)

{
  copy_source_word_block();
  DAT_00ffef14 = DAT_00ffef14 | 1;
  run_menu_metadata_service();
  draw_hud_footer();
  return;
}


