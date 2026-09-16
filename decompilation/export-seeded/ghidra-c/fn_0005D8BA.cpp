// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0005D8BA


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0005d8ba(void)

{
  short sVar1;
  
  _DAT_00ffdf90 = _DAT_00ffdf90 + -1;
  if (_DAT_00ffdf90 < 0) {
    _DAT_00ffdf90 = 4;
    _DAT_00ffdf92 = _DAT_00ffdf92 + 1;
    if (7 < _DAT_00ffdf92) {
      _DAT_00ffdf92 = 0;
    }
    sVar1 = _DAT_00ffdf92 << 1;
    _DAT_00ffd884 = *(undefined2 *)((int)&DAT_0005dce6 + (int)sVar1);
    _DAT_00ffd886 = *(undefined2 *)((int)&DAT_0005dce8 + (int)sVar1);
    _DAT_00ffd888 = *(undefined2 *)((int)&DAT_0005dcea + (int)sVar1);
    _DAT_00ffd88a = *(undefined2 *)((int)&DAT_0005dcec + (int)sVar1);
    _DAT_00ffd88c = *(undefined2 *)((int)&DAT_0005dcee + (int)sVar1);
    _DAT_00ffd88e = *(undefined2 *)((int)&DAT_0005dcf0 + (int)sVar1);
    _DAT_00ffd890 = *(undefined2 *)((int)&DAT_0005dcf2 + (int)sVar1);
    _DAT_00ffd892 = *(undefined2 *)((int)&DAT_0005dcf4 + (int)sVar1);
    DAT_00ffef14 = DAT_00ffef14 | 1;
  }
  else if (_DAT_00ffdf90 == 2) {
    FUN_0005b2e2();
    return;
  }
  return;
}


