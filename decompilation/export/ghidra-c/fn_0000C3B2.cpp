// Ghidra headless export
// Entry: 0x0000C3B2
// Name: write_single_word
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000E9C make_vdp_transfer_command


void write_single_word(void)

{
  word extraout_D1w;
  
  make_vdp_transfer_command();
  VDP_DATA = extraout_D1w;
  return;
}


