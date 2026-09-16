// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0005A59C


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0005a59c(void)

{
  if (_DAT_00fff150 <= _DAT_00ffdf88) {
    if (5 < _DAT_00fff14e) {
      return;
    }
    _DAT_00fff14a = *(undefined4 *)(&DAT_0005a602 + (short)(_DAT_00fff14e * 4));
    _DAT_00fff150 = _DAT_00ffdf88 + 8;
    _DAT_00fff14e = _DAT_00fff14e + 1;
  }
  FUN_00001378();
  return;
}


