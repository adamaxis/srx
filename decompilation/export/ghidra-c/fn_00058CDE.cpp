// Ghidra headless export
// Entry: 0x00058CDE
// Name: FUN_00058cde
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000E34 FUN_00000e34
//   0x00058B5E FUN_00058b5e
//   0x00058BB6 FUN_00058bb6
//   0x00058D32 FUN_00058d32


void FUN_00058cde(void)

{
  char in_D0b;
  
  DAT_00fff0d1 = in_D0b + DAT_00fff0d1;
  if (DAT_00fff0d1 < '\0') {
    DAT_00fff0d1 = 0;
    return;
  }
  if (-1 < (char)(DAT_00fff0d1 + -2)) {
    DAT_00fff0d1 = 1;
    return;
  }
  DAT_00fff0d0 = 0;
  if (DAT_00fff0d1 != '\0') {
    FUN_00058b5e();
    FUN_00058d32();
    return;
  }
  FUN_00000e34();
  FUN_00058b5e();
  FUN_00058bb6();
  return;
}


