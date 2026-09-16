// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0001E04A


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0001e04a(void)

{
  if (_DAT_00ffddc4 == 0x14) {
    FUN_0001dbaa();
  }
  else if ((_DAT_00ffdf40 == 2) && (_DAT_00ffe11e = 0x960, DAT_00ffe16f == '\0')) {
    DAT_00ffe16f = -1;
    FUN_000530de();
    FUN_0001df00();
    FUN_0001e0dc();
  }
  return;
}


