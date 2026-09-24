// Ghidra headless export
// Entry: 0x00017EB4
// Name: FUN_00017eb4
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000D81C FUN_0000d81c


void FUN_00017eb4(void)

{
  char cVar1;
  char extraout_D1b;
  
  cVar1 = FUN_0000d81c();
  DAT_00fff0a7 = '\x1d' - (extraout_D1b + cVar1);
  return;
}


