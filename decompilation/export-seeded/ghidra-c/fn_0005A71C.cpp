// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0005A71C


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0005a71c(void)

{
  int iVar1;
  
  if (_DAT_00ffdf88 < _DAT_00fff16a) {
    if (_DAT_00fff16c < 0x40) {
      return;
    }
  }
  else {
    if (7 < _DAT_00fff168) {
      return;
    }
    iVar1 = (int)(short)(_DAT_00fff168 << 3);
    _DAT_00fff164 = *(undefined4 *)((int)&PTR_DAT_0005a7aa + iVar1);
    _DAT_00fff16e = *(undefined2 *)(&DAT_0005a7ae + iVar1);
    _DAT_00fff16a = _DAT_00ffdf88 + *(short *)(&DAT_0005a7b0 + iVar1);
    _DAT_00fff16c = 0x200;
    _DAT_00fff168 = _DAT_00fff168 + 1;
  }
  _DAT_00fff16c = _DAT_00fff16c - 8;
  FUN_00001378();
  return;
}


