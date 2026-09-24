// Ghidra headless export
// Entry: 0x0000FF06
// Name: FUN_0000ff06
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void FUN_0000ff06(void)

{
  char in_D0b;
  
  if ((short)(*DAT_00fff10e + -7) < 0) {
    return;
  }
  DAT_00fff0d1 = in_D0b + DAT_00fff0d1;
  if (DAT_00fff0d1 < '\0') {
    DAT_00fff0d1 = '\x01';
  }
  else {
    if (DAT_00fff0d1 == '\x02') {
      DAT_00fff0d1 = 0;
      return;
    }
    if (DAT_00fff0d1 == '\0') {
      return;
    }
  }
  if (-1 < (char)((char)(*DAT_00fff10e + -7) - DAT_00fff0d0)) {
    return;
  }
  DAT_00fff0d1 = 0;
  return;
}


