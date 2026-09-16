// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0005D746


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0005d746(void)

{
  short sVar1;
  
  _DAT_00fffc0a = 1;
  FUN_0005da50();
  DAT_00ffe18a = 1;
  _DAT_00ffdd2a = 0x41;
  _DAT_00ffdd2c = 0x119;
  _DAT_00ffdd2e = 0x1a8;
  _DAT_00ffdd3c = 0;
  sVar1 = 0x150;
  _DAT_00ffdf92 = 9;
  do {
    FUN_00000bd2();
    if (_DAT_00ffdd3c != 0) {
      FUN_0005d8ba();
    }
    FUN_0005dac4();
    FUN_0005d938();
    if (_DAT_00ffdd2e == 0x160) {
LAB_0005d7c8:
      _DAT_00ffdd30 = _DAT_00ffdd30 + -1;
      if ((_DAT_00ffdd30 < 0) && (_DAT_00ffdd28 = _DAT_00ffdd28 + 1, (_DAT_00ffdd28 & 1) == 0)) {
        if (_DAT_00ffdd2a < 6) {
          FUN_000530d2();
        }
        _DAT_00ffdd2a = _DAT_00ffdd2a + -1;
        if (-1 < _DAT_00ffdd2a) {
          if (_DAT_00ffdd2a == 0) {
            FUN_000530d2();
            _DAT_00ffdd3c = 1;
          }
          _DAT_00ffdd0a = _DAT_00ffdd0a + -1;
          if (4 < _DAT_00ffdd2a) {
            FUN_000530de();
          }
        }
      }
    }
    else {
      _DAT_00ffdd2e = _DAT_00ffdd2e + -1;
      if (_DAT_00ffdd2e == 0x160) {
        _DAT_00ffdd30 = 0x14;
        goto LAB_0005d7c8;
      }
    }
    DAT_00ffef14 = DAT_00ffef14 | 0xe;
    sVar1 = sVar1 + -1;
    if (sVar1 == -1) {
      FUN_0000195c();
      FUN_0000131c();
      FUN_0005d886();
      sVar1 = 0x78;
      do {
        FUN_00000bd2();
        sVar1 = sVar1 + -1;
      } while (sVar1 != -1);
      FUN_0000195c();
      FUN_0000131c();
      return;
    }
  } while( true );
}


