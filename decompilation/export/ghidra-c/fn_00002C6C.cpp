// Ghidra headless export
// Entry: 0x00002C6C
// Name: FUN_00002c6c
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001406 prepare_title_resources
//   0x00002C92 FUN_00002c92
//   0x00002CB2 FUN_00002cb2
//   0x00016AFA FUN_00016afa
//   0x000566C2 FUN_000566c2


void FUN_00002c6c(void)

{
  prepare_title_resources();
  FUN_000566c2();
  if (DAT_00ffdf40 == 2) {
    FUN_00016afa();
  }
  FUN_00002c92();
  FUN_00002cb2();
  return;
}


