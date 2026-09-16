// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0005AF2A


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0005af2a(void)

{
  short *psVar1;
  short *psVar2;
  
  _DAT_00ffdd36 = _DAT_00ffdd36 + -1;
  if (_DAT_00ffdd36 < 0) {
    psVar1 = _DAT_00ffdd32 + 1;
    psVar2 = _DAT_00ffdd32 + 3;
    _DAT_00ffdd32 = psVar2;
    (**(code **)psVar1)();
    _DAT_00ffdd36 = *psVar2;
    if (_DAT_00ffdd36 == 0) {
      return;
    }
  }
  return;
}


