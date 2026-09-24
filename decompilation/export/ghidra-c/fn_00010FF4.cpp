// Ghidra headless export
// Entry: 0x00010FF4
// Name: FUN_00010ff4
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000F5C write_vdp_word_block_with_source_wrap


void FUN_00010ff4(void)

{
  short sVar1;
  short sVar2;
  
  sVar1 = 2;
  do {
    sVar2 = 0x4f;
    do {
      write_vdp_word_block_with_source_wrap();
      sVar2 = sVar2 + -1;
    } while (sVar2 != -1);
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  DAT_00fff112 = 0;
  return;
}


