// Ghidra headless export
// Entry: 0x000033BE
// Name: write_four_by_four_transfer_batch
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000F5C write_vdp_word_block_with_source_wrap


undefined8 write_four_by_four_transfer_batch(void)

{
  undefined4 in_D0;
  undefined4 in_D1;
  short sVar1;
  short sVar2;
  
  sVar2 = 3;
  do {
    sVar1 = 3;
    do {
      write_vdp_word_block_with_source_wrap();
      sVar1 = sVar1 + -1;
    } while (sVar1 != -1);
    sVar2 = sVar2 + -1;
  } while (sVar2 != -1);
  return CONCAT44(in_D0,in_D1);
}


