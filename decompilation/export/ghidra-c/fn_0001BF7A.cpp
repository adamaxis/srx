// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0001BF7A


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0001bf7a(void)

{
  short sVar1;
  
  sVar1 = _DAT_00fff11e + -1;
  if (sVar1 < 0) {
    if ((short)(_DAT_00fff11e + 7) < 0) {
      sVar1 = *(short *)(&DAT_0001bffc +
                        (short)(*(char *)((short)(_DAT_00fff118 * 0x14) + 0xff1106) * 2));
    }
    else {
      _DAT_00fff11e = sVar1;
      FUN_00017c3c();
      func_0x0001c012();
      DAT_00ffef14 = DAT_00ffef14 | 1;
      sVar1 = _DAT_00fff11e;
    }
  }
  _DAT_00fff11e = sVar1;
  return;
}


