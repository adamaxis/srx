// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00059F8E


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00059f8e(void)

{
  ushort uVar1;
  
  uVar1 = _DAT_00ffdf88 - _DAT_00fff11e;
  if (uVar1 < 0x169) {
    if (uVar1 == 0x168) {
      FUN_000530de();
      _DAT_00ffdd08 = 0;
      FUN_00000f1c();
      FUN_0005b32c();
      FUN_00001728();
    }
    else {
      if (0x11f < uVar1) {
        _DAT_00fff124 = _DAT_00fff124 + 0xaa;
        _DAT_00ffdd08 = _DAT_00fff124 >> 8;
        DAT_00ffef14 = DAT_00ffef14 | 4;
      }
      if (_DAT_00fff122 <= _DAT_00ffdf88) {
        _DAT_00fff122 =
             _DAT_00ffdf88 + *(short *)((int)&PTR_DAT_0005a084 + (int)(short)(_DAT_00fff120 * 6));
        FUN_00001728();
        _DAT_00fff120 = _DAT_00fff120 + 1;
        if (5 < _DAT_00fff120) {
          _DAT_00fff120 = 0;
        }
      }
    }
  }
  return;
}


