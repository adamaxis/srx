// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00000DFC


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00000dfc(void)

{
  undefined2 unaff_D2w;
  short unaff_D7w;
  
  FUN_00000e9c();
  do {
    _DAT_00c00000 = unaff_D2w;
    unaff_D7w = unaff_D7w + -1;
  } while (unaff_D7w != -1);
  return;
}


