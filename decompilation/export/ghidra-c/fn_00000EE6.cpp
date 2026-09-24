// Ghidra headless export
// Entry: 0x00000EE6
// Name: FUN_00000ee6
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000E34 reset_vdp_upload_state
//   0x00000F98 write_vdp_word_block


void FUN_00000ee6(void)

{
  if (0x4f < DAT_00ffdd02) {
    DAT_00ffdbf9 = 0;
  }
  if (DAT_00ffdd02 != 0xffff) {
    write_vdp_word_block();
    return;
  }
  reset_vdp_upload_state();
  return;
}


