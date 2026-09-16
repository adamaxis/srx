// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0001AA40


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0001aa40(void)

{
  char cVar1;
  byte bVar2;
  char extraout_D1b;
  int extraout_A0;
  bool bVar3;
  
  if (_DAT_00ff164a == 0) {
    return;
  }
  if (_DAT_00ffe0f2 == 0) {
    if (_DAT_00ffe082 == 0) {
      if ((_DAT_00ffdf8c == 0) || (_DAT_00ffe0bc != 0)) {
        DAT_00ff1642 = DAT_00ff1642 - 1;
        if ((short)((ushort)DAT_00ff1642 << 8) < 0) {
          cVar1 = FUN_0001a4ea();
          bVar2 = extraout_D1b + cVar1;
          if ((short)((ushort)bVar2 << 8) < 0) {
            bVar2 = 0;
          }
          *(byte *)(extraout_A0 + 4) = bVar2;
          *(char *)(extraout_A0 + 5) = *(char *)(extraout_A0 + 5) + '\x01';
          if (*(byte *)(extraout_A0 + 7) <= *(byte *)(extraout_A0 + 5)) {
            *(undefined1 *)(extraout_A0 + 5) = 0;
          }
        }
      }
      else {
        DAT_00ff1642 = DAT_00ff1642 - 1;
        if ((short)((ushort)DAT_00ff1642 << 8) < 0) {
          DAT_00ff1642 = DAT_00ff1644;
          DAT_00ff1643 = DAT_00ff1643 + 1;
          if (3 < DAT_00ff1643) {
            DAT_00ff1643 = 3;
          }
        }
      }
    }
    if (_DAT_00ffe082 == 0) {
      FUN_000061cc();
    }
    FUN_000013be();
    FUN_0001ab6e();
    FUN_0001ac6a();
    FUN_0001ad42();
    return;
  }
  DAT_00ff1642 = DAT_00ff1642 - 1;
  if ((short)((ushort)DAT_00ff1642 << 8) < 0) {
    DAT_00ff1642 = 5;
    DAT_00ff1643 = DAT_00ff1643 + 1;
    if (5 < DAT_00ff1643) {
      _DAT_00ffe082 = 0;
      _DAT_00ffe080 = 1;
      _DAT_00ff164a = 0;
      return;
    }
  }
  FUN_000061cc();
  FUN_000013be();
  if ((_DAT_00ffe084 == 0) && (_DAT_00ffe0b6 != 0)) {
    if ((_DAT_00ffe0c8 != 0) && (_DAT_00ffe0be = _DAT_00ffe056 << 2, 0x6aff < _DAT_00ffe0be)) {
      _DAT_00ffe0c8 = 0;
      _DAT_00ffe0b6 = 0;
      _DAT_00ffe0f4 = 1;
      FUN_0001a818();
      return;
    }
    if (_DAT_00ffe0b6 != 1) {
      _DAT_00ffe0be = _DAT_00ffe0c0 + _DAT_00ffe0be;
      bVar3 = false;
      FUN_0001ac50();
      if (!bVar3) {
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
  return;
}


