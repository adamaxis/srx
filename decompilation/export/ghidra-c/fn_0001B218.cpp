// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0001B218


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0001b218(void)

{
  _DAT_00ffe0fa = _DAT_00ffe0fa + -1;
  if (_DAT_00ffe0fa < 0) {
    _DAT_00ffe0fa = 3;
    _DAT_00ffe0f8 = _DAT_00ffe0f8 + 1;
    if (5 < _DAT_00ffe0f8) {
      _DAT_00ffe0f8 = 0;
    }
  }
  FUN_000061cc();
  FUN_000013be();
  return;
}


