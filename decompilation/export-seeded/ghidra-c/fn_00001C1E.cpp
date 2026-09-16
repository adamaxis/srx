// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00001C1E


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00001c1e(void)

{
  if (_DAT_00ffdd5a == 1) {
    _DAT_00ffdd5c = _DAT_00ffdd5c - 1;
    if ((short)_DAT_00ffdd5c < 0) {
      _DAT_00ffdd5a = 2;
      FUN_000530ea();
      FUN_00001df6();
    }
    else if ((_DAT_00ffdd5c & 0x1f) == 0) {
      FUN_000010d6();
      DAT_00ffef14 = DAT_00ffef14 | 1;
      return;
    }
  }
  return;
}


