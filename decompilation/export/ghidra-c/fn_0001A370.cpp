// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0001A370


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0001a370(void)

{
  ushort uVar2;
  uint uVar1;
  short sVar3;
  uint *extraout_A1;
  bool bVar4;
  
  if (_DAT_00ff164a == 0) {
    return;
  }
  bVar4 = _DAT_00ffe082 == 0;
  if (!bVar4) {
    if (_DAT_00ffe0f2 != 0) {
      return;
    }
    DAT_00ff1642 = DAT_00ff1642 - 1;
    if ((short)((ushort)DAT_00ff1642 << 8) < 0) {
      DAT_00ff1642 = 2;
      DAT_00ff1644 = DAT_00ff1644 - 1;
      if ((short)((ushort)DAT_00ff1644 << 8) < 0) {
        _DAT_00ffe082 = 0;
        _DAT_00ffe080 = 1;
        _DAT_00ff164a = 0;
        return;
      }
    }
    FUN_00000bd2();
    FUN_00000e9c();
    sVar3 = 0x1ff;
    do {
      _DAT_00c00000 = FUN_0001d3f4();
      _DAT_00c00000 = *extraout_A1 & _DAT_00c00000;
      sVar3 = sVar3 + -1;
    } while (sVar3 != -1);
    return;
  }
  FUN_0001a538();
  if (!bVar4) {
    if (((DAT_00ffdf84 != '\0') && (_DAT_00ffe076 == 0)) &&
       (_DAT_00ffe074 = _DAT_00ffe074 + -1, _DAT_00ffe074 < 0)) {
      _DAT_00ffe074 = FUN_00017e78();
      _DAT_00ffe07e = FUN_000561e8();
      if (-1 < _DAT_00ffe07e) {
        if (_DAT_00ffe07e == 2) {
          uVar1 = FUN_00017e78();
          _DAT_00ffe0c4 = (undefined2)(0xa00 / (uVar1 & 0xffff));
          _DAT_00ffe0c2 = 1;
          _DAT_00ffe0c6 = 0x11f0;
        }
        else if (_DAT_00ffe07e == 3) {
          uVar2 = (ushort)DAT_00fff0e7;
          if (uVar2 == 0) {
            uVar2 = 1;
          }
          _DAT_00ffe0c0 = (undefined2)(0x2800 / ((uint)uVar2 * 0xf0 & 0xffff));
          if (_DAT_00ffe0bc == 6) {
            _DAT_00ffe0b8 = 2;
          }
          else if (_DAT_00ffe0bc == 7) {
            _DAT_00ffe0b6 = 2;
          }
        }
        else {
          FUN_0001a54e();
          _DAT_00ffe04e = 2;
        }
      }
    }
    return;
  }
  FUN_0001a818();
  return;
}


