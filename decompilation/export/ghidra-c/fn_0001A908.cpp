// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0001A908


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0001a908(void)

{
  short sVar1;
  undefined1 *puVar2;
  
  _DAT_00ff164a = 1;
  sVar1 = _DAT_00ffe0e0;
  if (_DAT_00ffe0de == 0) {
    sVar1 = FUN_00019b40();
  }
  _DAT_00ffe0bc = sVar1;
  if (sVar1 < 0) {
    puVar2 = &DAT_0001aa10;
  }
  else {
    if (sVar1 == 0x45) {
      _DAT_00ff164a = 0;
      _DAT_00ffe080 = 1;
      return;
    }
    if (sVar1 == 6) {
      if (_DAT_00ffe0b8 == 0) {
        _DAT_00ffe0b8 = 1;
        _DAT_00ffe0ba = 0x47c0;
      }
    }
    else if ((sVar1 == 7) && (_DAT_00ffe0b6 == 0)) {
      _DAT_00ffe0b6 = 1;
      _DAT_00ffe0be = 0x4800;
    }
    puVar2 = *(undefined1 **)(&DAT_0001a9ce + (short)(sVar1 << 2));
  }
  DAT_00ff1644 = *puVar2;
  DAT_00ff1645 = puVar2[1];
  _DAT_00ff1646 = *(undefined4 *)(puVar2 + 2);
  _DAT_00ff1642 = 0;
  _DAT_00ff163e = 0x120;
  _DAT_00ff1640 = 0xd0;
  return;
}


