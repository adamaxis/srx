// Ghidra headless export
// Entry: 0x0001107E
// Name: FUN_0001107e
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001530 decode_and_upload_source_stream


void FUN_0001107e(void)

{
  if (DAT_00fff100 == '\0') {
    decode_and_upload_source_stream();
    return;
  }
  decode_and_upload_source_stream();
  return;
}


