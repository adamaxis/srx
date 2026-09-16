// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00026972


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00026972(void)

{
  if (DAT_00fff0d2 != '\0') {
    if (_DAT_00ffe0ee != 0) {
      _DAT_00ffe0f0 = _DAT_00ffe0f0 + -1;
      if (_DAT_00ffe0f0 < 0) {
        _DAT_00ffe0ee = 0;
      }
      else if (1 < (_DAT_00ffdf88 & 3)) {
        return;
      }
    }
    FUN_00001378();
  }
  return;
}


