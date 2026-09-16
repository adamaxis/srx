// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00001BAC


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00001bac(void)

{
  if (((DAT_00fff101 == '\a') && (_DAT_00ffcb40 == 0x40101040)) && (_DAT_00ffcb44 == 0x20401000)) {
    DAT_00fff101 = 0xff;
    return;
  }
  DAT_00fff101 = 0;
  return;
}


