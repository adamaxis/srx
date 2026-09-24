// Ghidra headless export
// Entry: 0x0000102A
// Name: FUN_0000102a
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000E9C make_vdp_transfer_command
//   0x00001056 clear_vdp_words


undefined4 FUN_0000102a(void)

{
  short sVar1;
  
  clear_vdp_words();
  make_vdp_transfer_command();
  sVar1 = 0x3ff;
  do {
    Ram00c00000 = 0x7ff07ff;
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  return 0x7ff07ff;
}


