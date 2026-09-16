// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x000026D0


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_000026d0(void)

{
  _DAT_00ffdd2e = _DAT_00ffdd2e + -1;
  if (_DAT_00ffdd2e < 0) {
    _DAT_00ffdd2e = 4;
    _DAT_00ffdd0e = _DAT_00ffdd0e + 1;
    DAT_00ffef14 = DAT_00ffef14 | 8;
  }
  return;
}


