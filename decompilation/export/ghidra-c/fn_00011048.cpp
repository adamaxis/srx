// Ghidra headless export
// Entry: 0x00011048
// Name: upload_resource_then_write_two_blocks
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000F5C write_vdp_word_block_with_source_wrap
//   0x0001107E upload_resource_stream


void upload_resource_then_write_two_blocks(void)

{
  upload_resource_stream();
  write_vdp_word_block_with_source_wrap();
  write_vdp_word_block_with_source_wrap();
  return;
}


