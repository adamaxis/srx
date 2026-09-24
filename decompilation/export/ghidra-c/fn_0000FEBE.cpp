// Ghidra headless export
// Entry: 0x0000FEBE
// Name: FUN_0000febe
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void FUN_0000febe(void)

{
  char in_D0b;
  short sVar1;
  
  sVar1 = *DAT_00fff10e;
  if (DAT_00fff0d1 == '\0') {
    if (-1 < (short)(sVar1 + -7)) {
      sVar1 = 6;
    }
  }
  else {
    sVar1 = sVar1 + -7;
  }
  DAT_00fff0d0 = in_D0b + DAT_00fff0d0;
  if (DAT_00fff0d0 < '\0') {
    DAT_00fff0d0 = (char)sVar1;
    return;
  }
  if (-1 < (char)((char)sVar1 - DAT_00fff0d0)) {
    return;
  }
  DAT_00fff0d0 = 0;
  return;
}


