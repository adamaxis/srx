// Ghidra headless export
// Entry: 0x00001CFC
// Name: FUN_00001cfc
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001530 decode_and_upload_source_stream
//   0x00001D58 FUN_00001d58


void FUN_00001cfc(void)

{
  decode_and_upload_source_stream();
  DAT_00fff1e2 = &UNK_00002090;
  DAT_00fff1e6 = 0;
  decode_and_upload_source_stream();
  decode_and_upload_source_stream();
  FUN_00001d58();
  return;
}


