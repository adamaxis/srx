// Ghidra headless export
// Entry: 0x00010F50
// Name: FUN_00010f50
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000F5C write_vdp_word_block_with_source_wrap
//   0x00001232 write_vdp_grid
//   0x00010918 prepare_menu_graphics
//   0x00010FF4 FUN_00010ff4
//   0x00011048 upload_resource_then_write_two_blocks


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00010f50(void)

{
  DAT_00fff0d0 = 6;
  DAT_00fff0d1 = 0;
  DAT_00fff098 = 0;
  _DAT_00fff0dc = 0;
  prepare_menu_graphics();
  write_vdp_grid();
  write_vdp_grid();
  write_vdp_grid();
  write_vdp_grid();
  FUN_00010ff4();
  upload_resource_then_write_two_blocks();
  write_vdp_word_block_with_source_wrap();
  return;
}


