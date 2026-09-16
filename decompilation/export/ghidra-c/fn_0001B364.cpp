// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0001B364


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0001b364(void)

{
  FUN_0001c3fa();
  _DAT_00ffe0b0 = _DAT_00ffe0b0 + -1;
  if (_DAT_00ffe0b0 < 0) {
    _DAT_00ffe0b0 = 0;
    _DAT_00ffe0ae = _DAT_00ffe0ae + 1;
    if (4 < _DAT_00ffe0ae) {
      _DAT_00ffe0ae = 0;
    }
  }
  FUN_000061cc();
  FUN_000013be();
  FUN_000013be();
  FUN_000061cc();
  FUN_000013be();
  FUN_000013be();
  return;
}


