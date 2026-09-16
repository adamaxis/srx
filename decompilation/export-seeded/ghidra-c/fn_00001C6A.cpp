// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00001C6A


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00001c6a(void)

{
  _DAT_00ffdd02 = 1;
  _DAT_00ffdd3c = _DAT_00ffdd3c + -1;
  if (_DAT_00ffdd3c < 0) {
    _DAT_00ffdd3c = 3;
    _DAT_00ffdd3e = _DAT_00ffdd3e + 1;
    if (5 < _DAT_00ffdd3e) {
      _DAT_00ffdd3e = 0;
    }
  }
  _DAT_00ffdd42 = _DAT_00ffdd42 + 4;
  FUN_00001378();
  _DAT_00ffd980 = 1;
  return;
}


