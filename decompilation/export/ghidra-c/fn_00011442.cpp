// Ghidra headless export
// Entry: 0x00011442
// Name: FUN_00011442
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000E34 reset_vdp_upload_state
//   0x000111D0 upload_menu_tile_block


void FUN_00011442(void)

{
  reset_vdp_upload_state();
  upload_menu_tile_block();
  return;
}


