// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00001872


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00001872(void)

{
  byte bVar1;
  
  _DAT_00fff9ec = _DAT_00fff9ec + 1;
  _DAT_00fff9f0 = 0;
  bVar1 = DAT_00ffef14 & 2;
  DAT_00ffef14 = DAT_00ffef14 & 0xfd;
  if (bVar1 != 0) {
    FUN_00000ee6();
    _DAT_00ffdd02 = 1;
  }
  bVar1 = DAT_00ffef14 & 1;
  DAT_00ffef14 = DAT_00ffef14 & 0xfe;
  if (bVar1 != 0) {
    FUN_00000ec0();
  }
  if (_DAT_00ffdfe8 == 0) {
    FUN_0005672c();
  }
  bVar1 = DAT_00ffef14 & 4;
  DAT_00ffef14 = DAT_00ffef14 & 0xfb;
  if (bVar1 != 0) {
    FUN_00000f1c();
  }
  bVar1 = DAT_00ffef14 & 8;
  DAT_00ffef14 = DAT_00ffef14 & 0xf7;
  if (bVar1 != 0) {
    FUN_00000f3c();
  }
  if (_DAT_00ffe100 != 0) {
    FUN_00002c92();
  }
  if (_DAT_00ffe102 != 0) {
    FUN_00002cb2();
  }
  if (_DAT_00ffdf2a != 0) {
    FUN_00006148();
  }
  if (_DAT_00ffdf30 != 0) {
    FUN_000061f4();
  }
  return;
}


