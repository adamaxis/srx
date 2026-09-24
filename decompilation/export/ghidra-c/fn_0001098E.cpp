// Ghidra headless export
// Entry: 0x0001098E
// Name: FUN_0001098e
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001232 upload_hud_tile_block


void FUN_0001098e(void)

{
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


