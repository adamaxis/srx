// Ghidra headless export
// Entry: 0x000026D0
// Name: FUN_000026d0
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void FUN_000026d0(void)

{
  DAT_00ffdd2e = DAT_00ffdd2e + -1;
  if (DAT_00ffdd2e < 0) {
    DAT_00ffdd2e = 4;
    DAT_00ffdd0e = DAT_00ffdd0e + 1;
    DAT_00ffef14 = DAT_00ffef14 | 8;
  }
  return;
}


