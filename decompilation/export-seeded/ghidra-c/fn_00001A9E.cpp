// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00001A9E


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00001a9e(void)

{
  ushort uVar1;
  
  DAT_00fff101 = 0;
  FUN_00001d7a();
  _DAT_00ffe00a = 0;
  _DAT_00ffdd0e = 0;
  FUN_0000195c();
  _DAT_00ffddc2 = 0;
  _DAT_00ffdd2c = 0x5a;
  _DAT_00ffdd2e = 0;
  _DAT_00ffdd3c = 0;
  _DAT_00ffdd5a = 0;
  _DAT_00ffdd5c = 0x5a;
  _DAT_00ffdd42 = 0x98;
  _DAT_00ffdd40 = 0;
  _DAT_00ffdd46 = 0;
  uVar1 = 0xe10;
  do {
    FUN_00000bd2();
    FUN_00001c1e();
    FUN_00002634();
    FUN_000026d0();
    FUN_0000195c();
    FUN_00001bea();
    FUN_00000bf2();
    _DAT_00ffdd2c = _DAT_00ffdd2c + -1;
    if ((_DAT_00ffdd2c < 0) && (_DAT_00fff1e6 < 0x26)) {
      _DAT_00ffdd40 = _DAT_00ffdd40 + -1;
      if (_DAT_00ffdd40 < 0) {
        _DAT_00ffdd40 = 1;
        FUN_00001e4e();
      }
      FUN_00001c6a();
      DAT_00ffef14 = DAT_00ffef14 | 2;
    }
    else {
      FUN_00000e34();
    }
    if (uVar1 < 0xc6c) {
      if ((uVar1 & 0x3f) < 10) {
        FUN_00001de0();
      }
      else {
        FUN_00001dca();
      }
      if (_DAT_00ffddc2 != 0) break;
    }
    uVar1 = uVar1 - 1;
  } while (uVar1 != 0xffff);
  FUN_0000195c();
  FUN_0000131c();
  FUN_00001bac();
  return;
}


