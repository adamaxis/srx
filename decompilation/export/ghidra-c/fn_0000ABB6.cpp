// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0000ABB6


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0000abb6(void)

{
  if ((((DAT_00ff0110 & 0x40) != 0) && (((DAT_00ff0210 & 1) == 0 || ((DAT_00ff0210 & 0x40) != 0))))
     && (((DAT_00ff0310 & 1) == 0 || ((DAT_00ff0310 & 0x40) != 0)))) {
    _DAT_00ffe166 = 0x3c;
  }
  return;
}


