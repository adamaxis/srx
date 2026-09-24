// Ghidra headless export
// Entry: 0x0001F38A
// Name: prepare_mode_resource_variant_a
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000F5C write_vdp_word_block_with_source_wrap


void prepare_mode_resource_variant_a(void)

{
  short sVar1;
  
  sVar1 = 0x4f;
  do {
    write_vdp_word_block_with_source_wrap();
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  return;
}


