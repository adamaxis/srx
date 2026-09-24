// Ghidra headless export
// Entry: 0x00010BC4
// Name: transfer_menu_asset_batch_eight_after_copy
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000F5C FUN_00000f5c
//   0x00001232 upload_hud_tile_block
//   0x00010918 prepare_menu_graphics


void transfer_menu_asset_batch_eight_after_copy(void)

{
  prepare_menu_graphics();
  FUN_00000f5c();
  upload_hud_tile_block();
  upload_hud_tile_block();
  upload_hud_tile_block();
  upload_hud_tile_block();
  upload_hud_tile_block();
  upload_hud_tile_block();
  upload_hud_tile_block();
  upload_hud_tile_block();
  VDP_CTRL = 0x8174;
  return;
}


