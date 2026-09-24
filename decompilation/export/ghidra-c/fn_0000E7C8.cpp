// Ghidra headless export
// Entry: 0x0000E7C8
// Name: FUN_0000e7c8
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000E8A0 FUN_0000e8a0
//   0x0000E9EE FUN_0000e9ee
//   0x0000EA36 FUN_0000ea36


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

ushort FUN_0000e7c8(void)

{
  char cVar1;
  ushort unaff_D5w;
  ushort unaff_D6w;
  bool bVar2;
  
  _DAT_00ffdf68 = 0;
  _DAT_00ffdfe2 = 0;
  if ((((DAT_00ffddd2 <= unaff_D5w >> 4) || (unaff_D5w >> 4 < DAT_00ffddd6)) ||
      (DAT_00ffddd4 <= unaff_D6w >> 4)) || (unaff_D6w >> 4 < DAT_00ffddd8)) {
    DAT_00ffef10._2_2_ = 0x7f;
    DAT_00ffef10._0_2_ = 0x7f;
    _DAT_00ffdfe2 = 0;
    _DAT_00ffdf68 = 0xffff;
    return 0x7f;
  }
  cVar1 = FUN_0000e8a0();
  DAT_00ffdf64 = (ushort)cVar1;
  if (DAT_00ffdf64 == 0) {
    return 0;
  }
  if (DAT_00ffdf64 != 0x7f) {
    bVar2 = false;
    if ((short)DAT_00ffdf64 < 0) {
      return DAT_00ffdf64;
    }
    if ((DAT_00ffdf64 & 0x40) == 0) {
      if ((DAT_00ffdf64 & 0x20) == 0) {
        FUN_0000e9ee();
        if (!bVar2) {
          return 0;
        }
        return 0x7f;
      }
    }
    else {
      _DAT_00ffdfe2 = 0xffff;
      bVar2 = DAT_00ffdf64 < 0x7e;
      if (DAT_00ffdf64 == 0x7e) {
        _DAT_00ffdfe2 = 0xffff;
        return 0x7f;
      }
      if ((DAT_00ffdf64 & 0x20) == 0) {
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


