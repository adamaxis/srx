// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x000173F2


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_000173f2(void)

{
  if (DAT_00ffe16f == '\0') {
    DAT_00fff0a0 = -1;
  }
  else {
    DAT_00fff0a0 = '\x01';
  }
  DAT_00fff0b0 = 0xff;
  FUN_0001745e();
  DAT_00fff0e3 = 0;
  DAT_00fff0b0 = 0;
  _DAT_00c00004 = 0x8134;
  FUN_00001406();
  if ((_DAT_00ffdf40 == 2) && (DAT_00fff0a0 == '\x01')) {
    DAT_00ffe191 = 1;
  }
  return;
}


