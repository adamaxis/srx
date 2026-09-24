// Ghidra headless export
// Entry: 0x0005DA50
// Name: setup_pre_title_resources
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000FFE FUN_00000ffe
//   0x0000102A FUN_0000102a
//   0x00001232 FUN_00001232
//   0x000012F6 FUN_000012f6
//   0x00001406 FUN_00001406
//   0x00001530 decode_and_upload_source_stream
//   0x000176BA FUN_000176ba
//   0x0005DAE8 FUN_0005dae8


void setup_pre_title_resources(void)

{
  FUN_00000ffe();
  FUN_0000102a();
  FUN_00001406();
  FUN_0005dae8();
  FUN_000176ba();
  decode_and_upload_source_stream();
  DAT_00ffef14 = DAT_00ffef14 | 0xd;
  FUN_000012f6();
  FUN_00001232();
  return;
}


