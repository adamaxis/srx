// Ghidra headless export
// Entry: 0x0005D996
// Name: FUN_0005d996
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001378 FUN_00001378


void FUN_0005d996(void)

{
  if (-1 < DAT_00ffdd30) {
    return;
  }
  if (DAT_00ffdd3c != 0) {
    return;
  }
  DAT_00ffe066 = DAT_00ffe066 + -1;
  if (DAT_00ffe066 < 0) {
    DAT_00ffe066 = 3;
    DAT_00ffe064 = DAT_00ffe064 + 1;
    if (5 < DAT_00ffe064) {
      DAT_00ffe064 = 0;
    }
  }
  FUN_00001378();
  FUN_00001378();
  return;
}


