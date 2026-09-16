// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0001BCE8


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0001bce8(void)

{
  if (_DAT_00ffe058 == 1) {
    DAT_00ff1594 = DAT_00ff1594 - 1;
    if (-1 < (short)((ushort)DAT_00ff1594 << 8)) {
LAB_0001bd50:
      FUN_00001378();
      return;
    }
    DAT_00ff1594 = 1;
    DAT_00ff1593 = DAT_00ff1593 + '\x01';
    if (DAT_00ff1593 < '\r') goto LAB_0001bd50;
    _DAT_00ffe058 = 2;
    _DAT_00ffe056 = 0x1340;
    _DAT_00ffe054 = 0xff0;
    _DAT_00ffe05c = 0xf;
    _DAT_00ffe052 = 0x3c;
    _DAT_00ffe05a = 0;
  }
  _DAT_00ffe052 = _DAT_00ffe052 + -1;
  if (-1 < _DAT_00ffe052) {
    _DAT_00ffe05c = _DAT_00ffe05c + -1;
    if (_DAT_00ffe05c < 0) {
      _DAT_00ffe05c = 0xf;
      _DAT_00ffe05a = _DAT_00ffe05a + 1;
      if (3 < _DAT_00ffe05a) goto LAB_0001bdb8;
    }
    _DAT_00ffe056 =
         0x1340 - (short)((uint)(*(short *)((int)&DAT_00015e56 +
                                           (int)(short)((0x3c - _DAT_00ffe052) * 3 & 0xfe)) * 0x1600
                                ) >> 0x10);
    _DAT_00ffe054 = _DAT_00ffe054 + -10;
    FUN_00001378();
    return;
  }
LAB_0001bdb8:
  _DAT_00ffe058 = 0;
  _DAT_00ffe0d6 = 0;
  if (_DAT_00ffe0c8 != 0) {
    _DAT_00ffe0f4 = 1;
  }
  FUN_0001a106();
  return;
}


