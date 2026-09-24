// Ghidra headless export
// Entry: 0x00017EE4
// Name: reset_menu_overlay_and_upload_two
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void reset_menu_overlay_and_upload_two(void)

{
  DAT_00fff0e4 = 0;
  write_vdp_grid();
  write_vdp_grid();
  return;
}


