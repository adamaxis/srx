// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0001669C


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

short FUN_0001669c(void)

{
  ushort uVar1;
  ushort *in_A0;
  
  if (in_A0[0x17] < 0x8000) {
    uVar1 = (*in_A0 >> 4) + 0x10;
  }
  else {
    uVar1 = (*in_A0 >> 4) - 0x10;
  }
  return ((in_A0[2] >> 9) - 1) * _DAT_00ffddea + (uVar1 >> 5) * 2;
}


