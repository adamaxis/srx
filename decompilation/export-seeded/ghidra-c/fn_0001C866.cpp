// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0001C866


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0001c866(void)

{
  _DAT_00ffe066 = _DAT_00ffe066 + -1;
  if (_DAT_00ffe066 < 0) {
    _DAT_00ffe066 = 3;
    _DAT_00ffe064 = _DAT_00ffe064 + 1;
    if (0xb < _DAT_00ffe064) {
      if (_DAT_00ffe060 < 0x120) {
        _DAT_00ff1588 = &DAT_0001c288;
        DAT_00ff158c = 0;
      }
      else {
        _DAT_00ff1584 = &DAT_0001c28e;
        DAT_00ff158d = 0;
      }
      _DAT_00ffe0d6 = 0;
      FUN_00019e32();
      _DAT_00ffe05e = 0;
      return;
    }
  }
  FUN_00001378();
  return;
}


