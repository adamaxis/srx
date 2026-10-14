// Ghidra headless export
// Entry: 0x00010984
// Name: transfer_menu_asset_batch_nine_optional
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00010918 prepare_menu_graphics


void transfer_menu_asset_batch_nine_optional(void)

{
  VDP_CTRL = 0x8134;
  prepare_menu_graphics();
  write_vdp_grid();
  write_vdp_grid();
  write_vdp_grid();
  write_vdp_grid();
  write_vdp_grid();
  write_vdp_grid();
  write_vdp_grid();
  write_vdp_grid();
  write_vdp_grid();
  if (DAT_00fff0af != '\0') {
    write_vdp_grid();
  }
  VDP_CTRL = 0x8174;
  return;
}


