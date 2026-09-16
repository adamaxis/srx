// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x000148AA


short FUN_000148aa(void)

{
  short unaff_D5w;
  short unaff_D7w;
  int unaff_A4;
  
  do {
    unaff_D5w = *(short *)(&DAT_000148ca + unaff_D7w) + unaff_D5w;
  } while (*(char *)(unaff_A4 + unaff_D5w) == '\0');
  return *(short *)(&DAT_000148ca + unaff_D7w);
}


