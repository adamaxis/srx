// Ghidra headless export
// Entry: 0x00058BB6
// Name: FUN_00058bb6
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001232 upload_hud_tile_block
//   0x0000C0DA render_title_text_source


void FUN_00058bb6(void)

{
  if (((DAT_00fff0b7 < '\0') || (DAT_00fff0b7 == '\0')) || (DAT_00fff0b7 != '\x01')) {
    upload_hud_tile_block();
    render_title_text_source();
  }
  return;
}


