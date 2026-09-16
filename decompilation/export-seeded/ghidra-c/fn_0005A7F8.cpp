// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0005A7F8


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0005a7f8(void)

{
  int iVar1;
  
  if (_DAT_00ffdf88 < _DAT_00fff176) {
    if (_DAT_00fff178 < 0x40) {
      return;
    }
  }
  else {
    if (7 < _DAT_00fff174) {
      return;
    }
    iVar1 = (int)(short)(_DAT_00fff174 << 3);
    _DAT_00fff170 = *(undefined4 *)((int)&PTR_DAT_0005a888 + iVar1);
    _DAT_00fff17a = *(undefined2 *)(&DAT_0005a88c + iVar1);
    _DAT_00fff176 = _DAT_00ffdf88 + *(short *)(&DAT_0005a88e + iVar1);
    _DAT_00fff178 = 0x200;
    _DAT_00fff174 = _DAT_00fff174 + 1;
  }
  _DAT_00fff178 = _DAT_00fff178 - 0xb;
  FUN_00001378();
  return;
}


