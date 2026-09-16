// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0005A3BE


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0005a3be(void)

{
  int iVar1;
  
  if (_DAT_00fff138 <= _DAT_00ffdf88) {
    if (4 < _DAT_00fff136) {
      return;
    }
    iVar1 = (int)(short)(_DAT_00fff136 * 10);
    _DAT_00fff132 = *(undefined4 *)((int)&PTR_DAT_0005a482 + iVar1);
    _DAT_00fff138 = _DAT_00ffdf88 + *(short *)(&DAT_0005a486 + iVar1);
    if (*(short *)(&DAT_0005a488 + iVar1) != 0) {
      _DAT_00fff13a = *(short *)(&DAT_0005a488 + iVar1);
    }
    _DAT_00fff13c = *(short *)(&DAT_0005a48a + iVar1);
    _DAT_00fff136 = _DAT_00fff136 + 1;
    if ((4 < _DAT_00fff136) && (_DAT_00fff136 = 1, 0x167 < (ushort)(_DAT_00ffdf88 - _DAT_00fff11e)))
    {
      _DAT_00fff136 = 5;
      return;
    }
  }
  _DAT_00fff13a = _DAT_00fff13c + _DAT_00fff13a;
  FUN_00001378();
  return;
}


