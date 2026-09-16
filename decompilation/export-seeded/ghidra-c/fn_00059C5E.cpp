// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00059C5E


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00059c5e(void)

{
  DAT_00ffe18b = 0xff;
  FUN_0000195c();
  _DAT_00ffddc2 = 0;
  FUN_000530ea();
  FUN_0005af62();
  if (_DAT_00ffddc2 == 0) {
    FUN_00053126();
    FUN_000530ea();
    _DAT_00ffddc2 = 0;
    FUN_00059d7c();
    if (_DAT_00ffddc2 == 0) {
      FUN_00053126();
      FUN_000530ea();
      _DAT_00ffddc2 = 0;
      FUN_0005aa96();
      if (_DAT_00ffddc2 == 0) {
        _DAT_00ffddc2 = 0;
        FUN_0005ad1a();
        _DAT_00ffddc2 = 0;
        FUN_0000131c();
        DAT_00fffbf8 = 2;
        FUN_00059d12();
      }
    }
  }
  FUN_0000195c();
  FUN_0000131c();
  FUN_00053126();
  return;
}


