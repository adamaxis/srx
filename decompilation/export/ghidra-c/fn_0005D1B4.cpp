// Ghidra headless export
// Entry: 0x0005D1B4
// Name: FUN_0005d1b4
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x000530EA FUN_000530ea
//   0x00053126 prepare_mode_entry_service
//   0x00059C34 adjust_runner_source_byte_triplet


void FUN_0005d1b4(void)

{
  prepare_mode_entry_service();
  FUN_000530ea();
  adjust_runner_source_byte_triplet();
  DAT_00fff0be = 0xc;
  func_0x0005d1f6();
  prepare_mode_entry_service();
  DAT_00ffe172 = 0xff;
  return;
}


