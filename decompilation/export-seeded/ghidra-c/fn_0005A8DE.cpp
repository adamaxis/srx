// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0005A8DE


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0005a8de(void)

{
  if (_DAT_00fff182 <= _DAT_00ffdf88) {
    if ((ushort)(_DAT_00ffdf88 - _DAT_00fff11e) < 6) {
      return;
    }
    if (2 < _DAT_00fff180) {
      return;
    }
    _DAT_00fff17c = *(undefined4 *)((int)&PTR_DAT_0005a958 + (int)(short)(_DAT_00fff180 * 6));
    _DAT_00fff182 =
         _DAT_00ffdf88 + *(short *)((int)&PTR_DAT_0005a95c + (int)(short)(_DAT_00fff180 * 6));
    _DAT_00fff180 = _DAT_00fff180 + 1;
  }
  _DAT_00fff184 = _DAT_00fff184 + 4;
  FUN_00001378();
  return;
}


