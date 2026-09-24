// Ghidra headless export
// Entry: 0x0005B29A
// Name: clear_title_intro_display_buffer
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000E9C make_vdp_transfer_command


undefined4 clear_title_intro_display_buffer(void)

{
  short sVar1;
  
  make_vdp_transfer_command();
  sVar1 = 0x3ff;
  do {
    Ram00c00000 = 0x7ff07ff;
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  return 0x7ff07ff;
}


