// Ghidra headless export
// Entry: 0x00010952
// Name: reset_mode_display_buffers
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000BD2 wait_for_vblank
//   0x00000E34 reset_vdp_upload_state
//   0x00000F1C write_transfer_header
//   0x00000F3C write_fixed_transfer_pair
//   0x00000FFE clear_and_fill_plane_a
//   0x0000102A clear_and_fill_plane_b


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void reset_mode_display_buffers(void)

{
  clear_and_fill_plane_a();
  clear_and_fill_plane_b();
  wait_for_vblank();
  reset_vdp_upload_state();
  _DAT_00ffdd08 = 0;
  _DAT_00ffdd0c = 0;
  write_transfer_header();
  write_fixed_transfer_pair();
  return;
}


