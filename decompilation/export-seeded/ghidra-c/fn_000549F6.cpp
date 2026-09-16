// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x000549F6


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_000549f6(void)

{
  if (DAT_00fff0a0 == '\0') {
    _DAT_00ffd8fc = 0xcc;
  }
  else if (DAT_00fff0a0 < '\x01') {
    _DAT_00ffd8fc = 0xc0;
  }
  else {
    _DAT_00ffd8fc = 0xc;
  }
  DAT_00ffef14 = DAT_00ffef14 | 1;
  return;
}


