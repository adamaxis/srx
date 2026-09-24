// Ghidra headless export
// Entry: 0x00010984
// Name: FUN_00010984
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00010918 FUN_00010918


void FUN_00010984(void)

{
  VDP_CTRL = 0x8134;
  FUN_00010918();
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


