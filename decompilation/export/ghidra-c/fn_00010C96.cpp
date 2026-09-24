// Ghidra headless export
// Entry: 0x00010C96
// Name: transfer_menu_asset_batch_eight
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001232 write_vdp_grid
//   0x00010918 prepare_menu_graphics


void transfer_menu_asset_batch_eight(void)

{
  prepare_menu_graphics();
  write_vdp_grid();
  write_vdp_grid();
  write_vdp_grid();
  write_vdp_grid();
  write_vdp_grid();
  write_vdp_grid();
  write_vdp_grid();
  VDP_CTRL = 0x8174;
  return;
}


