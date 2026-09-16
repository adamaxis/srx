// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0005B00A


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0005b00a(void)

{
  if (-0x201 < _DAT_00ffdd08) {
    if ((_DAT_00ffdd08 < -399) && (_DAT_00fff18a == 0)) {
      _DAT_00ffdd08 = FUN_0005b32c();
      _DAT_00fff18a = 1;
    }
    _DAT_00ffdd08 = _DAT_00ffdd08 + -2;
    DAT_00ffef14 = DAT_00ffef14 | 4;
  }
  return;
}


