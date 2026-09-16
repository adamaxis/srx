// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x000166D0


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

short FUN_000166d0(void)

{
  ushort uVar1;
  ushort *in_A0;
  
  if ((in_A0[0x17] < 0x4000) || (0xbfff < in_A0[0x17])) {
    uVar1 = (in_A0[2] >> 4) - 0x10;
  }
  else {
    uVar1 = (in_A0[2] >> 4) + 0x10;
  }
  return ((uVar1 >> 5) - 1) * _DAT_00ffddea + (*in_A0 >> 9) * 2;
}


