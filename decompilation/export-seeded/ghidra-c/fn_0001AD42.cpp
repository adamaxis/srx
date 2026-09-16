// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0001AD42


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0001ad42(void)

{
  if ((_DAT_00ffe084 != 0) || (_DAT_00ffe0c2 == 0)) {
    return;
  }
  if (DAT_00fff0a3 == '\0') {
    _DAT_00ffe0c2 = 0;
  }
  _DAT_00ffe0c6 = _DAT_00ffe0c4 + _DAT_00ffe0c6;
  _DAT_00ffe0b4 = _DAT_00ffe0b4 + -1;
  if (_DAT_00ffe0b4 < 0) {
    _DAT_00ffe0b4 = 3;
    _DAT_00ffe0b2 = _DAT_00ffe0b2 + 1;
    if (3 < _DAT_00ffe0b2) {
      _DAT_00ffe0b2 = 0;
    }
  }
  FUN_000061cc();
  FUN_000013be();
  if (_DAT_00ffe0b2 != 0) {
    return;
  }
  return;
}


