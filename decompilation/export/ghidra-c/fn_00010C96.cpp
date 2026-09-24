// Ghidra headless export
// Entry: 0x00010C96
// Name: FUN_00010c96
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001232 upload_hud_tile_block
//   0x00010918 FUN_00010918


void FUN_00010c96(void)

{
  FUN_00010918();
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


