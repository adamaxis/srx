// Ghidra headless export
// Entry: 0x00058B5E
// Name: FUN_00058b5e
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001232 FUN_00001232
//   0x00057C46 FUN_00057c46


void FUN_00058b5e(void)

{
  FUN_00057c46();
  if (((DAT_00fff0b7 < '\0') || (DAT_00fff0b7 == '\0')) || (DAT_00fff0b7 != '\x01')) {
    FUN_00001232();
  }
  return;
}


