// Ghidra headless export
// Entry: 0x00000E18
// Name: write_word_stream
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000E9C make_vdp_transfer_command


void write_word_stream(void)

{
  short unaff_D7w;
  word *extraout_A0;
  word *pwVar1;
  
  make_vdp_transfer_command();
  pwVar1 = extraout_A0;
  do {
    VDP_DATA = *pwVar1;
    unaff_D7w = unaff_D7w + -1;
    pwVar1 = pwVar1 + 1;
  } while (unaff_D7w != -1);
  return;
}


