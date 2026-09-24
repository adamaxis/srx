// Ghidra headless export
// Entry: 0x000176BA
// Name: FUN_000176ba
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000F5C write_vdp_word_block_with_source_wrap
//   0x00001530 decode_and_upload_source_stream


void FUN_000176ba(void)

{
  write_vdp_word_block_with_source_wrap();
  decode_and_upload_source_stream();
  return;
}


