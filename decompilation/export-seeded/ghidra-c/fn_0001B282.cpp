// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0001B282


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0001b282(void)

{
  _DAT_00ffe0da = _DAT_00ffe0da + -1;
  if (_DAT_00ffe0da < 0) {
    _DAT_00ffe0da = 0xc;
    _DAT_00ffe0dc = _DAT_00ffe0dc + 1;
    if (_DAT_00ffe0dc == 6) {
      FUN_00019632();
      FUN_00017bc0();
    }
    if (0xb < _DAT_00ffe0dc) {
      FUN_00019aee();
      _DAT_00ffe0de = 0;
      FUN_0001792e();
      FUN_0001a908();
      return;
    }
  }
  FUN_00001378();
  return;
}


