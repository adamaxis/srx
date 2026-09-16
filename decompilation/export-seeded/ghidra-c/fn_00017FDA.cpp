// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00017FDA


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00017fda(void)

{
  short sVar1;
  
  if (_DAT_00fff106 < 0) {
    sVar1 = FUN_00000d3a();
    if (sVar1 != 0) {
      return;
    }
    _DAT_00fff106 = 7;
    _DAT_00fff09a = FUN_00000d3a();
    sVar1 = FUN_00000d3a();
    if (sVar1 == 0) {
      _DAT_00fff09a = _DAT_00fff09a + 0x1c;
    }
    _DAT_00fff09a = _DAT_00fff09a * 2;
  }
  else if (_DAT_00fff106 < 1) {
    FUN_00017fb4();
    goto LAB_00018148;
  }
  FUN_00017fb4();
  FUN_00001232();
  if ((_DAT_00fff106 != 2) && (_DAT_00fff106 != 7)) {
    FUN_00000d3a();
    FUN_000011f6();
    if ((_DAT_00fff106 == 4) || (_DAT_00fff106 == 5)) {
      FUN_00000d3a();
      FUN_000011f6();
      if (_DAT_00fff106 == 5) {
        FUN_00001232();
      }
    }
  }
LAB_00018148:
  _DAT_00fff106 = _DAT_00fff106 + -1;
  return;
}


