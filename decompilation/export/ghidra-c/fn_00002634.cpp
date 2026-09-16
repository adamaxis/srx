// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00002634


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00002634(void)

{
  if (_DAT_00ffdd5a != 2) {
    return;
  }
  _DAT_00ffdf90 = _DAT_00ffdf90 + -1;
  if (_DAT_00ffdf90 < 0) {
    _DAT_00ffdf90 = 4;
    _DAT_00ffdf92 = _DAT_00ffdf92 + 1;
    if (6 < _DAT_00ffdf92) {
      _DAT_00ffdf92 = 0;
    }
    _DAT_00c00004 = 0;
    _DAT_00c00000 = *(undefined2 *)((int)&DAT_00002700 + (int)(short)(_DAT_00ffdf92 << 1));
    return;
  }
  return;
}


