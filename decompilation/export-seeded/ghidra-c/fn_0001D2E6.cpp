// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0001D2E6


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0001d2e6(void)

{
  short *psVar1;
  
  for (psVar1 = _DAT_00ffdf7a; *psVar1 != -0x8000; psVar1 = psVar1 + 4) {
    FUN_0001d33a();
  }
  _DAT_00ffdf82 = _DAT_00ffdf82 + -1;
  if (_DAT_00ffdf82 < 0) {
    _DAT_00ffdf82 = 0;
    _DAT_00ffdf70 = _DAT_00ffdf70 + 1;
  }
  if (_DAT_00ffdf70 <= _DAT_00ffdf78) {
    return;
  }
  return;
}


