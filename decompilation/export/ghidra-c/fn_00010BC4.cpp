// Ghidra headless export
// Entry: 0x00010BC4
// Name: FUN_00010bc4
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000F5C FUN_00000f5c
//   0x00001232 upload_hud_tile_block
//   0x00010918 FUN_00010918


void FUN_00010bc4(void)

{
  FUN_00010918();
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


