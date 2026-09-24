// Ghidra headless export
// Entry: 0x00012F50
// Name: FUN_00012f50
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001232 upload_hud_tile_block
//   0x0000C0DA render_title_text_source
//   0x00013000 FUN_00013000


void FUN_00012f50(void)

{
  FUN_00013000();
  render_title_text_source();
  upload_hud_tile_block();
  return;
}


