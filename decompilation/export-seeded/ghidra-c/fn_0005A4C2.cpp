// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0005A4C2


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0005a4c2(void)

{
  ushort uVar1;
  
  if (_DAT_00fff146 < 4) {
    if (_DAT_00fff148 <= _DAT_00ffdf88) {
      _DAT_00fff13e = *(undefined4 *)((int)&DAT_0005a562 + (int)(short)(_DAT_00fff146 * 4));
      uVar1 = _DAT_00fff146 * 4 + 4;
      if (0xf < uVar1) {
        uVar1 = 0;
      }
      _DAT_00fff142 = *(undefined4 *)((int)&DAT_0005a562 + (int)(short)uVar1);
      _DAT_00fff148 = _DAT_00ffdf88 + 6;
      _DAT_00fff146 = _DAT_00fff146 + 1;
      if (3 < _DAT_00fff146) {
        _DAT_00fff146 = 0;
      }
    }
    FUN_00001378();
    FUN_00001378();
  }
  return;
}


