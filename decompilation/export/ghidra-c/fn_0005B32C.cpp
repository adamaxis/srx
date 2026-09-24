// Ghidra headless export
// Entry: 0x0005B32C
// Name: fill_title_intro_word_block
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000E9C make_vdp_transfer_command


undefined8 fill_title_intro_word_block(void)

{
  undefined4 in_D0;
  undefined4 in_D1;
  short extraout_D1w;
  short unaff_D7w;
  short sVar1;
  
  do {
    make_vdp_transfer_command();
    sVar1 = unaff_D7w;
    do {
      VDP_DATA = 0x7ff;
      sVar1 = sVar1 + -1;
    } while (sVar1 != -1);
  } while (extraout_D1w != 0);
  return CONCAT44(in_D0,in_D1);
}


