// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0001A5A0


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0001a5a0(void)

{
  short extraout_D1w;
  int extraout_A0;
  
  if (_DAT_00ffe076 == 2) {
LAB_0001a64c:
    _DAT_00ffe07c = _DAT_00ffe07c + -1;
    if (-1 < _DAT_00ffe07c) {
      FUN_000061cc();
      _DAT_00ffe078 = _DAT_00ffe078 + -6;
      _DAT_00ffe07a = _DAT_00ffe07a + 0x1a;
LAB_0001a62a:
      FUN_000013be();
      return;
    }
    FUN_000562ec();
    if (_DAT_00ffe07e != 0) {
      _DAT_00ffe07c = 0x10;
      _DAT_00ffe076 = 4;
LAB_0001a6e6:
      _DAT_00ffe07c = _DAT_00ffe07c + -1;
      if (_DAT_00ffe07c < 0) {
        FUN_0001b680();
        _DAT_00ffe076 = 0;
        return;
      }
      if (_DAT_00ffe0d6 == 0) {
        FUN_0001b654();
        return;
      }
      return;
    }
    FUN_000530ea();
    _DAT_00ffe07c = 8;
    _DAT_00ffe076 = 3;
    _DAT_00ffe0fc = extraout_D1w;
    if (DAT_00fff0d2 != '\0') {
      _DAT_00ffe0f0 = 8;
      _DAT_00ffe0ee = 1;
    }
  }
  else if (_DAT_00ffe076 != 3) {
    if (_DAT_00ffe076 == 5) goto LAB_0001a6f8;
    if (_DAT_00ffe076 == 4) goto LAB_0001a6e6;
    if (_DAT_00ffe076 == 6) {
      _DAT_00ffe07c = _DAT_00ffe07c + -1;
      if (_DAT_00ffe07c < 0) {
        FUN_0005668c();
        _DAT_00ffe076 = 0;
        return;
      }
      FUN_000061cc();
      _DAT_00ffe078 = _DAT_00ffe078 + -2;
      _DAT_00ffe07a = _DAT_00ffe07a + -0xe;
      goto LAB_0001a62a;
    }
    _DAT_00ffe07c = _DAT_00ffe07c + -1;
    if (-1 < _DAT_00ffe07c) {
      FUN_000061cc();
      _DAT_00ffe078 = _DAT_00ffe078 + 6;
      _DAT_00ffe07a = _DAT_00ffe07a + -2;
      goto LAB_0001a62a;
    }
    _DAT_00ffe076 = 2;
    _DAT_00ffe07c = 0x1e;
    goto LAB_0001a64c;
  }
  _DAT_00ffe07c = _DAT_00ffe07c + -1;
  if (-1 < _DAT_00ffe07c) {
    if (_DAT_00ffe0fc < 0) {
      FUN_00001378();
      FUN_00001378();
      return;
    }
    return;
  }
  _DAT_00ffe076 = 5;
  _DAT_00ffe07c = 0x10;
LAB_0001a6f8:
  _DAT_00ffe07c = _DAT_00ffe07c + -1;
  if (_DAT_00ffe07c < 0) {
    FUN_0001b680();
    _DAT_00ffe076 = 0;
    return;
  }
  if (_DAT_00ffe0d6 == 0) {
    FUN_0001b7d0();
    *(undefined **)(extraout_A0 + 8) = &DAT_0001c2ae;
    *(undefined **)(extraout_A0 + 4) = &DAT_0001c2a8;
    return;
  }
  return;
}


