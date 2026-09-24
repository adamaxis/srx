// Ghidra headless export
// Entry: 0x00006148
// Name: FUN_00006148
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000F5C write_vdp_word_block_with_source_wrap


void FUN_00006148(void)

{
  short sVar1;
  
  sVar1 = DAT_00ffdf2a + -1;
  do {
    write_vdp_word_block_with_source_wrap();
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  DAT_00ffdf2a = 0;
  return;
}


