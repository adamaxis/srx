// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00059D7C


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00059d7c(void)

{
  short sVar1;
  
  FUN_0000c0c2();
  _DAT_00ffdd32 = &DAT_0005adf0;
  _DAT_00ffdd36 = DAT_0005adf0;
  FUN_00001406();
  _DAT_00ffdd08 = -0x40;
  _DAT_00ffdd0c = 0;
  _DAT_00ffdd0a = -0x20;
  _DAT_00ffdd0e = 0;
  DAT_00ffef14 = DAT_00ffef14 | 0xc;
  _DAT_00ffdd46 = 0x138;
  _DAT_00ffdd48 = 0xf1;
  _DAT_00ffdd02 = 1;
  FUN_0005a37c();
  _DAT_00ffd980 = 0;
  DAT_00ffef14 = DAT_00ffef14 | 2;
  FUN_000012f6();
  sVar1 = 0xa0;
  do {
    FUN_00000bd2();
    FUN_0000195c();
    FUN_00000bf2();
    FUN_0005af2a();
    if (_DAT_00ffdd08 != 0) {
      if (_DAT_00ffdd46 != 0x178) {
        _DAT_00ffdd46 = _DAT_00ffdd46 + 1;
      }
      _DAT_00ffdd08 = _DAT_00ffdd08 + 1;
      DAT_00ffef14 = DAT_00ffef14 | 4;
    }
    if ((_DAT_00ffdd0a != 0) && ((_DAT_00ffdf88 & 1) == 0)) {
      if (_DAT_00ffdd48 != 0xd1) {
        _DAT_00ffdd48 = _DAT_00ffdd48 + -1;
      }
      _DAT_00ffdd0a = _DAT_00ffdd0a + 1;
      DAT_00ffef14 = DAT_00ffef14 | 8;
    }
    _DAT_00ffdd02 = 1;
    FUN_0005a37c();
    _DAT_00ffd980 = 0;
    DAT_00ffef14 = DAT_00ffef14 | 2;
  } while ((_DAT_00ffddc2 == 0) && (sVar1 = sVar1 + -1, sVar1 != -1));
  _DAT_00fff11e = _DAT_00ffdf88;
  FUN_00059f7a();
  FUN_0005a0a4();
  FUN_0005a244();
  FUN_0005a3b0();
  FUN_0005a4b4();
  FUN_0005a572();
  FUN_0005a61a();
  FUN_0005a70e();
  FUN_0005a7ea();
  FUN_0005a8c8();
  FUN_0005a1ea();
  FUN_0000195c();
  sVar1 = 0x1f0;
  do {
    FUN_00000bd2();
    FUN_00059f8e();
    FUN_0005a0b2();
    FUN_0000195c();
    FUN_00000bf2();
    FUN_0005af2a();
    FUN_0005a1ea();
    if (sVar1 == 0x164) {
      FUN_0005ae3a();
      FUN_0005adfe();
    }
  } while ((_DAT_00ffddc2 == 0) && (sVar1 = sVar1 + -1, sVar1 != -1));
  return;
}


