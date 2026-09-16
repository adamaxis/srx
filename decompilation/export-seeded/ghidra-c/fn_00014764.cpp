// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00014764


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00014764(void)

{
  ushort uVar1;
  short sVar2;
  short sVar3;
  ushort unaff_D5w;
  short unaff_D6w;
  short unaff_D7w;
  int unaff_A4;
  
  sVar2 = *(short *)(&DAT_000148ca + unaff_D6w);
  sVar3 = *(short *)(&DAT_000148ca + unaff_D7w);
  uVar1 = sVar3 + unaff_D5w;
  if ((((-1 < (short)uVar1) && ((uVar1 & 0x3f) <= _DAT_00ffddea)) &&
      (uVar1 >> 6 <= _DAT_00ffddd4 >> 5)) && (*(char *)(unaff_A4 + (short)uVar1) == '\0')) {
    sVar2 = *(short *)(&DAT_000148ca + (short)(unaff_D6w + 2U & 6));
    sVar3 = *(short *)(&DAT_000148ca + (short)(unaff_D7w + 2U & 6));
  }
  while( true ) {
    unaff_D5w = sVar2 + unaff_D5w;
    if (((short)unaff_D5w < 0) || (_DAT_00ffddea < (unaff_D5w & 0x3f))) {
      return;
    }
    if (_DAT_00ffddd4 >> 5 < unaff_D5w >> 6) {
      return;
    }
    if (*(char *)(unaff_A4 + (short)unaff_D5w) != '\0') break;
    if (*(char *)(unaff_A4 + (short)(sVar3 + unaff_D5w)) == '\0') {
      return;
    }
  }
  return;
}


