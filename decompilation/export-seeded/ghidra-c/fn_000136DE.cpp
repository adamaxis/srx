// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x000136DE


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_000136de(void)

{
  _DAT_00ffddc4 = FUN_00053126();
  _DAT_00ffdf40 = 0;
  DAT_00fff0a0 = 0xff;
  _DAT_00ffdf8e = 0xffff;
  if (_DAT_00ffddc4 == 6) {
    _DAT_00fffc10 = 0;
    _DAT_00ffe10e = 1;
  }
  _DAT_00fffc0e = (ushort)(_DAT_00ffddc4 == 6);
  return;
}


