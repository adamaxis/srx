// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0005B100


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0005b100(void)

{
  if (-0x1b9 < _DAT_00ffdd08) {
    if (_DAT_00fff192 <= _DAT_00ffdf88) {
      _DAT_00fff18c = *(undefined4 *)((int)&PTR_DAT_0005b180 + (int)(short)(_DAT_00fff190 * 4));
      _DAT_00fff192 = _DAT_00ffdf88 + 6;
      _DAT_00fff190 = _DAT_00fff190 + 1;
      if (5 < _DAT_00fff190) {
        _DAT_00fff190 = 0;
      }
    }
    FUN_00001378();
  }
  return;
}


