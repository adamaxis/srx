// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0000BCAE


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0000bcae(void)

{
  ushort *in_A0;
  
  if ((((_DAT_00ffddc4 == 5) && (0x11ff < in_A0[2])) && (in_A0[2] < 0x1f80)) && (0x25ff < *in_A0)) {
    if (*in_A0 < 0x2800) {
      in_A0[2] = in_A0[2] - 8;
      return;
    }
    if ((0x3bff < *in_A0) && (*in_A0 < 0x3e00)) {
      in_A0[2] = in_A0[2] + 8;
      return;
    }
  }
  return;
}


