// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0005B056


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0005b056(void)

{
  if (_DAT_00ffdd08 < -0x1af) {
    if (_DAT_00ffdd08 == -0x1b0) {
      FUN_0005aa1e();
    }
    if (_DAT_00ffdd0e < 0x78) {
      _DAT_00ffdd0e = _DAT_00ffdd0e + 1;
      DAT_00ffef14 = DAT_00ffef14 | 8;
    }
  }
  return;
}


