// Ghidra headless export
// Entry: 0x000176EA
// Name: render_menu_footer
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001358 copy_source_word_block
//   0x00017F8E draw_hud_footer
//   0x00019DF2 FUN_00019df2


void render_menu_footer(void)

{
  copy_source_word_block();
  DAT_00ffef14 = DAT_00ffef14 | 1;
  FUN_00019df2();
  draw_hud_footer();
  return;
}


