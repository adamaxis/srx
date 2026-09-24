// Ghidra headless export
// Entry: 0x000118C6
// Name: FUN_000118c6
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void FUN_000118c6(void)

{
  char in_D0b;
  
  DAT_00fff0d0 = in_D0b + DAT_00fff0d0;
  if ((char)(DAT_00fff0d0 + -2) < '\0') {
    DAT_00fff0d0 = 5;
    DAT_00fff112 = 0xff;
    return;
  }
  if (DAT_00fff0d0 == '\x06') {
    DAT_00fff0d0 = '\x02';
  }
  return;
}


