// Ghidra headless export
// Entry: 0x000176EA
// Name: FUN_000176ea
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001358 copy_source_word_block
//   0x00017F8E FUN_00017f8e
//   0x00019DF2 FUN_00019df2


void FUN_000176ea(void)

{
  copy_source_word_block();
  DAT_00ffef14 = DAT_00ffef14 | 1;
  FUN_00019df2();
  FUN_00017f8e();
  return;
}


