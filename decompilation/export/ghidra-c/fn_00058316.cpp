// Ghidra headless export
// Entry: 0x00058316
// Name: FUN_00058316
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001232 upload_hud_tile_block
//   0x0000C0DA render_title_text_source


void FUN_00058316(void)

{
  undefined4 extraout_A1;
  
  if ((short)(DAT_00fff0d0 - 4) < 0) {
    render_title_text_source();
    render_title_text_source();
    upload_hud_tile_block(extraout_A1);
  }
  render_title_text_source();
  return;
}


