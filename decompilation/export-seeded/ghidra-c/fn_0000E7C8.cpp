// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0000E7C8


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

ushort FUN_0000e7c8(void)

{
  char cVar1;
  ushort unaff_D5w;
  ushort unaff_D6w;
  bool bVar2;
  
  _DAT_00ffdf68 = 0;
  _DAT_00ffdfe2 = 0;
  if ((((_DAT_00ffddd2 <= unaff_D5w >> 4) || (unaff_D5w >> 4 < _DAT_00ffddd6)) ||
      (_DAT_00ffddd4 <= unaff_D6w >> 4)) || (unaff_D6w >> 4 < _DAT_00ffddd8)) {
    _DAT_00ffef12 = 0x7f;
    _DAT_00ffef10 = 0x7f;
    _DAT_00ffdfe2 = 0;
    _DAT_00ffdf68 = 0xffff;
    return 0x7f;
  }
  cVar1 = FUN_0000e8a0();
  _DAT_00ffdf64 = (ushort)cVar1;
  if (_DAT_00ffdf64 == 0) {
    return 0;
  }
  if (_DAT_00ffdf64 != 0x7f) {
    bVar2 = false;
    if ((short)_DAT_00ffdf64 < 0) {
      return _DAT_00ffdf64;
    }
    if ((_DAT_00ffdf64 & 0x40) == 0) {
      if ((_DAT_00ffdf64 & 0x20) == 0) {
        FUN_0000e9ee();
        if (!bVar2) {
          return 0;
        }
        return 0x7f;
      }
    }
    else {
      _DAT_00ffdfe2 = 0xffff;
      bVar2 = _DAT_00ffdf64 < 0x7e;
      if (_DAT_00ffdf64 == 0x7e) {
        _DAT_00ffdfe2 = 0xffff;
        return 0x7f;
      }
      if ((_DAT_00ffdf64 & 0x20) == 0) {
        _DAT_00ffdfe2 = 0xffff;
        return 0x7f;
      }
    }
    FUN_0000ea36();
    if (bVar2) {
      return 0x7f;
    }
    return 0;
  }
  return 0x7f;
}


