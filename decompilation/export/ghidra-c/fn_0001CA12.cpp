// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0001CA12


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0001ca12(void)

{
  _DAT_00ffe070 = _DAT_00ffe070 + -1;
  if (_DAT_00ffe070 < 0) {
    _DAT_00ffe06e = 0;
    _DAT_00ffe0d6 = 0;
    FUN_00019e32();
    return;
  }
  _DAT_00ffe072 = _DAT_00ffe072 + 1;
  _DAT_00ffe06a = _DAT_00ffe06c + _DAT_00ffe06a;
  _DAT_00ffe068 = _DAT_00ffe068 + -0x1a;
  FUN_00001378();
  return;
}


