// Ghidra headless export
// Entry: 0x00001CFC
// Name: load_title_resources
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001530 decode_and_upload_source_stream
//   0x00001D58 prepare_title_sprite_list


void load_title_resources(void)

{
  decode_and_upload_source_stream();
  DAT_00fff1e2 = &UNK_00002090;
  DAT_00fff1e6 = 0;
  decode_and_upload_source_stream();
  decode_and_upload_source_stream();
  prepare_title_sprite_list();
  return;
}


