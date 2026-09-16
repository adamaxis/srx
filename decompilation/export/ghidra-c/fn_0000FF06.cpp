// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0000FF06


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0000ff06(void)

{
  char in_D0b;
  
  if ((short)(*_DAT_00fff10e + -7) < 0) {
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
  if (-1 < (char)((char)(*_DAT_00fff10e + -7) - DAT_00fff0d0)) {
    return;
  }
  DAT_00fff0d1 = 0;
  return;
}


