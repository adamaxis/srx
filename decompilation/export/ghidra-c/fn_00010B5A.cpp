// Ghidra headless export
// Entry: 0x00010B5A
// Name: transfer_menu_asset_batch_four
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001232 upload_hud_tile_block
//   0x00010918 prepare_menu_graphics


void transfer_menu_asset_batch_four(void)

{
  prepare_menu_graphics();
  upload_hud_tile_block();
  upload_hud_tile_block();
  upload_hud_tile_block();
  upload_hud_tile_block();
  VDP_CTRL = 0x8174;
  return;
}


