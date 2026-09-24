// Ghidra headless export
// Entry: 0x000549F6
// Name: FUN_000549f6
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void FUN_000549f6(void)

{
  if (DAT_00fff0a0 == '\0') {
    DAT_00ffd8fc = 0xcc;
  }
  else if (DAT_00fff0a0 < '\x01') {
    DAT_00ffd8fc = 0xc0;
  }
  else {
    DAT_00ffd8fc = 0xc;
  }
  DAT_00ffef14 = DAT_00ffef14 | 1;
  return;
}


