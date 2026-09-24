// Ghidra headless export
// Entry: 0x0001BCE8
// Name: FUN_0001bce8
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001378 FUN_00001378
//   0x0001A106 FUN_0001a106


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0001bce8(void)

{
  if (DAT_00ffe058 == 1) {
    DAT_00ff1594 = DAT_00ff1594 - 1;
    if (-1 < (short)((ushort)DAT_00ff1594 << 8)) {
LAB_0001bd50:
      FUN_00001378();
      return;
    }
    DAT_00ff1594 = 1;
    DAT_00ff1593 = DAT_00ff1593 + '\x01';
    if (DAT_00ff1593 < '\r') goto LAB_0001bd50;
    DAT_00ffe058 = 2;
    DAT_00ffe056 = 0x1340;
    DAT_00ffe054 = 0xff0;
    DAT_00ffe05c = 0xf;
    DAT_00ffe052 = 0x3c;
    DAT_00ffe05a = 0;
  }
  DAT_00ffe052 = DAT_00ffe052 + -1;
  if (-1 < DAT_00ffe052) {
    DAT_00ffe05c = DAT_00ffe05c + -1;
    if (DAT_00ffe05c < 0) {
      DAT_00ffe05c = 0xf;
      DAT_00ffe05a = DAT_00ffe05a + 1;
      if (3 < DAT_00ffe05a) goto LAB_0001bdb8;
    }
    DAT_00ffe056 = 0x1340 - (short)((uint)(*(short *)((int)&DAT_00015e56 +
                                                     (int)(short)((0x3c - DAT_00ffe052) * 3 & 0xfe))
                                          * 0x1600) >> 0x10);
    DAT_00ffe054 = DAT_00ffe054 + -10;
    FUN_00001378();
    return;
  }
LAB_0001bdb8:
  DAT_00ffe058 = 0;
  _DAT_00ffe0d6 = 0;
  if (_DAT_00ffe0c8 != 0) {
    _DAT_00ffe0f4 = 1;
  }
  FUN_0001a106();
  return;
}


