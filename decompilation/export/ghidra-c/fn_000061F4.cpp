// Ghidra headless export
// Entry: 0x000061F4
// Name: FUN_000061f4
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000F5C write_vdp_word_block_with_source_wrap


void FUN_000061f4(void)

{
  short sVar1;
  
  sVar1 = DAT_00ffdf30 + -1;
  do {
    write_vdp_word_block_with_source_wrap();
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  DAT_00ffdf30 = 0;
  return;
}


