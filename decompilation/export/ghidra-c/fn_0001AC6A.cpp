// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0001AC6A


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0001ac6a(void)

{
  if ((_DAT_00ffe084 != 0) || (_DAT_00ffe0b8 == 0)) {
    return;
  }
  if ((_DAT_00ffe0c8 != 0) && (_DAT_00ffe0ba = _DAT_00ffe056 << 2, 0x6aff < _DAT_00ffe0ba)) {
    _DAT_00ffe0b8 = 0;
    _DAT_00ffe0c8 = 0;
    _DAT_00ffe0f4 = 1;
    FUN_0001a818();
    return;
  }
  if (_DAT_00ffe0b8 != 1) {
    _DAT_00ffe0ba = _DAT_00ffe0c0 + _DAT_00ffe0ba;
    FUN_0001ac50();
  }
  _DAT_00ffe0b4 = _DAT_00ffe0b4 + -1;
  if (_DAT_00ffe0b4 < 0) {
    _DAT_00ffe0b4 = 6;
    _DAT_00ffe0b2 = _DAT_00ffe0b2 + 1;
    if (3 < _DAT_00ffe0b2) {
      _DAT_00ffe0b2 = 0;
    }
  }
  FUN_000061cc();
  FUN_000013be();
  return;
}


