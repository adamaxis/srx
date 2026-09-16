// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00001BEA


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00001bea(void)

{
  if (((_DAT_00ffdd22 & 0x7f00) != 0) && (DAT_00fff101 < 8)) {
    (&DAT_00ffcb40)[(short)DAT_00fff101] = (byte)(_DAT_00ffdd22 >> 8) & 0x7f;
    DAT_00fff101 = DAT_00fff101 + '\x01';
  }
  return;
}


