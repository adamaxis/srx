// Ghidra headless export
// Entry: 0x00010F50
// Name: FUN_00010f50
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000F5C FUN_00000f5c
//   0x00001232 upload_hud_tile_block
//   0x00010918 prepare_menu_graphics
//   0x00010FF4 FUN_00010ff4
//   0x00011048 FUN_00011048


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00010f50(void)

{
  DAT_00fff0d0 = 6;
  DAT_00fff0d1 = 0;
  DAT_00fff098 = 0;
  _DAT_00fff0dc = 0;
  prepare_menu_graphics();
  upload_hud_tile_block();
  upload_hud_tile_block();
  upload_hud_tile_block();
  upload_hud_tile_block();
  FUN_00010ff4();
  FUN_00011048();
  FUN_00000f5c();
  return;
}


