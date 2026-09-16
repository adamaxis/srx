// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0005A0B2


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0005a0b2(void)

{
  if ((_DAT_00fff126 < 0xc) && (_DAT_00fff128 <= _DAT_00ffdf88)) {
    if (_DAT_00fff126 < 4) {
      _DAT_00fff128 =
           _DAT_00ffdf88 + *(short *)((int)&PTR_DAT_0005a1d6 + (int)(short)(_DAT_00fff126 * 6));
      FUN_00001728();
      _DAT_00fff126 = _DAT_00fff126 + 1;
    }
    else {
      if (_DAT_00fff126 == 4) {
        FUN_000530de();
      }
      if (_DAT_00fff126 == 4) {
        FUN_0005b2c2();
        _DAT_00fff126 = _DAT_00fff126 + 1;
        _DAT_00fff128 = _DAT_00ffdf88 + 9;
      }
      else if (_DAT_00fff128 <= _DAT_00ffdf88) {
        FUN_0005b2e2();
        _DAT_00fff128 = _DAT_00ffdf88 + 9;
        _DAT_00fff126 = _DAT_00fff126 + 1;
        if (_DAT_00fff126 == 9) {
          FUN_0005a582();
        }
        if (0xb < _DAT_00fff126) {
          FUN_0005b32c();
        }
      }
    }
  }
  return;
}


