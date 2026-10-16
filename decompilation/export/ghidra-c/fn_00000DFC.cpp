// Ghidra headless export
// Entry: 0x00000DFC
// Name: write_repeated_words
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000E9C make_vdp_transfer_command


void write_repeated_words(void)

{
  word unaff_D2w;
  short unaff_D7w;
  
  make_vdp_transfer_command();
  do {
    VDP_DATA = unaff_D2w;
    unaff_D7w = unaff_D7w + -1;
  } while (unaff_D7w != -1);
  return;
}


