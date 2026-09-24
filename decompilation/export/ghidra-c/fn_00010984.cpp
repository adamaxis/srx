// Ghidra headless export
// Entry: 0x00010984
// Name: transfer_menu_asset_batch_nine_optional
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00010918 prepare_menu_graphics


void transfer_menu_asset_batch_nine_optional(void)

{
  VDP_CTRL = 0x8134;
  prepare_menu_graphics();
  upload_hud_tile_block();
  upload_hud_tile_block();
  upload_hud_tile_block();
  upload_hud_tile_block();
  upload_hud_tile_block();
  upload_hud_tile_block();
  upload_hud_tile_block();
  upload_hud_tile_block();
  upload_hud_tile_block();
  if (DAT_00fff0af != '\0') {
    upload_hud_tile_block();
  }
  VDP_CTRL = 0x8174;
  return;
}


