// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00014902


short FUN_00014902(void)

{
  ushort in_D0w;
  ushort in_D1w;
  
  return (in_D1w >> 4 & 0x7fe0) * 2 + (in_D0w >> 9);
}


