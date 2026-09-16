// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0005A252


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0005a252(void)

{
  if (_DAT_00fff12e < 0xc) {
    if (_DAT_00fff130 <= _DAT_00ffdf88) {
      if (_DAT_00fff12e < 4) {
        _DAT_00fff12a = *(undefined4 *)((int)&PTR_PTR_0005a398 + (int)(short)(_DAT_00fff12e * 6));
        _DAT_00fff130 = _DAT_00ffdf88 + *(short *)(&DAT_0005a39c + (short)(_DAT_00fff12e * 6));
        _DAT_00fff12e = _DAT_00fff12e + 1;
      }
      else {
        if (_DAT_00fff12e == 4) {
          FUN_000530de();
        }
        if (_DAT_00fff12e == 4) {
          _DAT_00fff146 = 4;
          FUN_0005b2c2();
          _DAT_00fff12e = _DAT_00fff12e + 1;
          _DAT_00fff130 = _DAT_00ffdf88 + 9;
        }
        else if (_DAT_00fff130 <= _DAT_00ffdf88) {
          FUN_0005b2e2();
          _DAT_00fff130 = _DAT_00ffdf88 + 9;
          _DAT_00fff12e = _DAT_00fff12e + 1;
          if (_DAT_00fff12e == 9) {
            FUN_0005a582();
          }
          if (0xb < _DAT_00fff12e) {
            FUN_0005b32c();
          }
        }
      }
    }
    FUN_00001378();
  }
  return;
}


