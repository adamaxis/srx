// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0005A628


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0005a628(void)

{
  int iVar1;
  
  if (_DAT_00ffdf88 < _DAT_00fff15c) {
    if (_DAT_00fff15e < _DAT_00fff162) {
      return;
    }
  }
  else {
    if (6 < _DAT_00fff15a) {
      return;
    }
    iVar1 = (int)(short)(_DAT_00fff15a * 10);
    _DAT_00fff156 = *(undefined4 *)((int)&PTR_DAT_0005a6c8 + iVar1);
    _DAT_00fff160 = *(undefined2 *)(&DAT_0005a6cc + iVar1);
    _DAT_00fff162 = *(ushort *)(&DAT_0005a6ce + iVar1);
    _DAT_00fff15c = _DAT_00ffdf88 + *(short *)(&DAT_0005a6d0 + iVar1);
    _DAT_00fff15e = 0x200;
    _DAT_00fff15a = _DAT_00fff15a + 1;
  }
  _DAT_00fff15e = _DAT_00fff15e - 9;
  FUN_00001378();
  return;
}


