// Ghidra headless export
// Entry: 0x00011442
// Name: reset_and_upload_menu_tile_block
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000E34 reset_vdp_upload_state
//   0x000111D0 upload_menu_tile_block


void reset_and_upload_menu_tile_block(void)

{
  reset_vdp_upload_state();
  upload_menu_tile_block();
  return;
}


