// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x000537FA


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_000537fa(void)

{
  undefined1 *in_A1;
  int unaff_A3;
  
  FUN_00053248();
  while ((char)in_A1[(short)(_DAT_00fff118 * 0x14) + 7] < '\0') {
    in_A1 = &DAT_00ff1100;
  }
  FUN_00001378();
  *(undefined1 *)(unaff_A3 + -5) = 0;
  _DAT_00ffd980 = 1;
  DAT_00ffef14 = DAT_00ffef14 | 2;
  return;
}


