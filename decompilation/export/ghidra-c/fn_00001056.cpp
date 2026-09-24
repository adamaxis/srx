// Ghidra headless export
// Entry: 0x00001056
// Name: clear_vdp_words
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000E9C make_vdp_transfer_command


undefined2 clear_vdp_words(void)

{
  short sVar1;
  
  make_vdp_transfer_command();
  sVar1 = 0xf;
  do {
    VDP_DATA = 0;
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  return 0;
}


