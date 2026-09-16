// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x000266E4


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_000266e4(void)

{
  undefined2 *unaff_A5;
  
  FUN_00024e20();
  _DAT_00ffe192 = *unaff_A5;
  _DAT_00ffe19a = *unaff_A5;
  _DAT_00ffe194 = unaff_A5[1];
  _DAT_00ffe19c = unaff_A5[1];
  if ((&DAT_00ff1100)[(short)(_DAT_00fff118 * 0x14)] == '\0') {
    _DAT_00ffe19e = 0xe;
    _DAT_00ffe1a0 = 8;
  }
  else {
    _DAT_00ffe19e = 0xc;
    _DAT_00ffe1a0 = 10;
  }
  _DAT_00ffe196 = 0;
  _DAT_00ffe198 = 0;
  _DAT_00ffe1a2 = 0;
  _DAT_00ffe1a4 = 0;
  _DAT_00ffe1a8 = 1;
  _DAT_00ffe1a6 = 1;
  FUN_00026696();
  _DAT_00ffe1aa = &DAT_00ff8700;
  _DAT_00ffe1ae = (undefined1 *)0xff9700;
  FUN_00024e20();
  _DAT_00ffe1ae = &DAT_00ffb740;
  FUN_00024e20();
  _DAT_00ffb740 = _DAT_00ffb740 + -1;
  _DAT_00ffdd28 = 0x11e;
  _DAT_00ffdd2a = 0xe4;
  return;
}


