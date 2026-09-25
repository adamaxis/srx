// Ghidra headless export
// Entry: 0x000127B0
// Name: refresh_menu_panel
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000C0DA render_encoded_title_text
//   0x000111D0 upload_menu_tile_block
//   0x000127FC write_single_grid


void refresh_menu_panel(void)

{
  upload_menu_tile_block();
  DAT_00fff114 = DAT_00fff114 ^ 0xff;
  write_single_grid();
  render_encoded_title_text();
  return;
}


