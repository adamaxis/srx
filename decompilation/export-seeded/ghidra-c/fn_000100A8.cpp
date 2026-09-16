// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x000100A8


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_000100a8(void)

{
  short sVar1;
  short sVar2;
  short in_D1w;
  
  _DAT_00fff096 = in_D1w + _DAT_00fff096;
  if (_DAT_00fff098 < _DAT_00fff096) {
    _DAT_00fff096 = _DAT_00fff098;
  }
  if (_DAT_00fff096 == 0) {
    _DAT_00fff096 = 1;
  }
  *(char *)(_DAT_00fffc04 + 0x9d) = (char)_DAT_00fff096;
  FUN_0000c216();
  FUN_00010172();
  FUN_000011c0();
  sVar2 = _DAT_00fff098;
  sVar1 = _DAT_00fff096;
  FUN_0001055e();
  _DAT_00fff098 = sVar2;
  _DAT_00fff096 = sVar1;
  FUN_00010156();
  FUN_000105e6();
  return;
}


