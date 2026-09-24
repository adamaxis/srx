// Ghidra headless export
// Entry: 0x0005D172
// Name: FUN_0005d172
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x000530EA FUN_000530ea
//   0x00053126 prepare_mode_entry_service


void FUN_0005d172(void)

{
  prepare_mode_entry_service();
  FUN_000530ea();
  FUN_00059c34();
  DAT_00fff0be = 7;
  func_0x0005d1f6();
  prepare_mode_entry_service();
  DAT_00ffe172 = 0xff;
  return;
}


