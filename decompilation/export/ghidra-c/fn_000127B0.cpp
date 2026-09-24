// Ghidra headless export
// Entry: 0x000127B0
// Name: refresh_menu_panel
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000C0DA render_title_text_source
//   0x000111D0 upload_menu_tile_block
//   0x000127FC FUN_000127fc


void refresh_menu_panel(void)

{
  upload_menu_tile_block();
  DAT_00fff114 = DAT_00fff114 ^ 0xff;
  FUN_000127fc();
  render_title_text_source();
  return;
}


