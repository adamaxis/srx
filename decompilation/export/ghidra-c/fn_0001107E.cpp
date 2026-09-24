// Ghidra headless export
// Entry: 0x0001107E
// Name: upload_resource_stream
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001530 decode_and_upload_source_stream


void upload_resource_stream(void)

{
  if (DAT_00fff100 == '\0') {
    decode_and_upload_source_stream();
    return;
  }
  decode_and_upload_source_stream();
  return;
}


