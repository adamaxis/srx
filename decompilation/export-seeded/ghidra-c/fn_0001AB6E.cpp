// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0001AB6E


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0001ab6e(void)

{
  bool bVar1;
  
  if ((_DAT_00ffe084 != 0) || (_DAT_00ffe0b6 == 0)) {
    return;
  }
  if ((_DAT_00ffe0c8 != 0) && (_DAT_00ffe0be = _DAT_00ffe056 << 2, 0x6aff < _DAT_00ffe0be)) {
    _DAT_00ffe0c8 = 0;
    _DAT_00ffe0b6 = 0;
    _DAT_00ffe0f4 = 1;
    FUN_0001a818();
    return;
  }
  if (_DAT_00ffe0b6 != 1) {
    _DAT_00ffe0be = _DAT_00ffe0c0 + _DAT_00ffe0be;
    bVar1 = false;
    FUN_0001ac50();
    if (!bVar1) {
      FUN_000563cc();
      return;
    }
  }
  _DAT_00ffe0b4 = _DAT_00ffe0b4 + -1;
  if (_DAT_00ffe0b4 < 0) {
    _DAT_00ffe0b4 = 8;
    _DAT_00ffe0b2 = _DAT_00ffe0b2 + 1;
    if (0x19 < _DAT_00ffe0b2) {
      _DAT_00ffe0b2 = 0;
    }
  }
  FUN_000061cc();
  FUN_000013be();
  return;
}


