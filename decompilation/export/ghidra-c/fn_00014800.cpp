// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00014800


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00014800(void)

{
  ushort unaff_D5w;
  short unaff_D6w;
  short unaff_D7w;
  int unaff_A4;
  
  do {
    unaff_D5w = *(short *)(&DAT_000148ca + unaff_D6w) + unaff_D5w;
    if ((((short)unaff_D5w < 0) || (_DAT_00ffddea < (unaff_D5w & 0x3f))) ||
       (_DAT_00ffddd4 >> 5 < unaff_D5w >> 6)) {
      return;
    }
    if (*(char *)(unaff_A4 + (short)unaff_D5w) != '\0') {
      return;
    }
  } while (*(char *)(unaff_A4 + (short)(*(short *)(&DAT_000148ca + unaff_D7w) + unaff_D5w)) != '\0')
  ;
  return;
}


