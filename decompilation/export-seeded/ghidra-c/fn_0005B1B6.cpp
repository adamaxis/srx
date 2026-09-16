// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0005B1B6


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0005b1b6(void)

{
  short sVar1;
  
  if (0x225 < (ushort)(_DAT_00ffdf88 - _DAT_00fff11e)) {
    if ((_DAT_00fff196 <= _DAT_00ffdf88) && (_DAT_00fff194 < 7)) {
      sVar1 = _DAT_00fff194 * 6;
      _DAT_00fff19c = *(short *)(&DAT_0005b270 + sVar1);
      _DAT_00fff19e = *(short *)(&DAT_0005b272 + sVar1);
      _DAT_00fff196 = _DAT_00ffdf88 + *(short *)(&DAT_0005b274 + sVar1);
      _DAT_00fff194 = _DAT_00fff194 + 1;
    }
    _DAT_00fff198 = _DAT_00fff19c + _DAT_00fff198;
    _DAT_00fff19a = _DAT_00fff19e + _DAT_00fff19a;
    FUN_00001378();
  }
  return;
}


