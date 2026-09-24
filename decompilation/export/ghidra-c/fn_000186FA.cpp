// Ghidra headless export
// Entry: 0x000186FA
// Name: FUN_000186fa
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001530 decode_and_upload_source_stream
//   0x00001728 FUN_00001728
//   0x00017708 FUN_00017708
//   0x000194AC FUN_000194ac
//   0x00026958 FUN_00026958


void FUN_000186fa(void)

{
  FUN_00017708();
  DAT_00fff0d4 = 0;
  FUN_000194ac();
  decode_and_upload_source_stream();
  FUN_00026958();
  FUN_00001728();
  FUN_00001728();
  return;
}


