// Ghidra headless export
// Entry: 0x00011454
// Name: reset_menu_upload_cursor_and_upload
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001232 upload_hud_tile_block


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void reset_menu_upload_cursor_and_upload(void)

{
  _DAT_00fff116 = 0;
  upload_hud_tile_block();
  return;
}


