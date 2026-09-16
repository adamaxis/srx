// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0001A48A


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0001a48a(void)

{
  _DAT_00ffe0d2 = _DAT_00ffe0d2 + -1;
  if (_DAT_00ffe0d2 < 0) {
    _DAT_00ffe0d2 = 2;
    _DAT_00ffe0d0 = _DAT_00ffe0d0 + 1;
    if (2 < _DAT_00ffe0d0) {
      _DAT_00ffe0d4 = 0;
      return;
    }
  }
  FUN_000061cc();
  FUN_000013be();
  return;
}


