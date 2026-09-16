// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0001E41E


undefined1 FUN_0001e41e(void)

{
  ushort unaff_D5w;
  ushort unaff_D6w;
  
  return (&DAT_00ff1700)[(short)((unaff_D5w >> 9) + (unaff_D6w >> 4 & 0x7fe0) * 2)];
}


