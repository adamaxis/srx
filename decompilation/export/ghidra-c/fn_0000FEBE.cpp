// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0000FEBE


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0000febe(void)

{
  char in_D0b;
  short sVar1;
  
  sVar1 = *_DAT_00fff10e;
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


