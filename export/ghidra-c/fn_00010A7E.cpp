// Ghidra headless export
// Entry: 0x00010A7E
// Name: transfer_menu_asset_batch_four_post_prepare
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00010918 prepare_menu_graphics


void transfer_menu_asset_batch_four_post_prepare(void)

{
  prepare_menu_graphics();
  VDP_CTRL = 0x8134;
  write_vdp_grid();
  write_vdp_grid();
  write_vdp_grid();
  write_vdp_grid();
  VDP_CTRL = 0x8174;
  return;
}


