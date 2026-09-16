// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00016D8A


void FUN_00016d8a(void)

{
  short unaff_D6w;
  short *extraout_A0;
  short *extraout_A1;
  undefined1 in_CF;
  
  FUN_00016e08();
  if ((bool)in_CF) {
    return;
  }
  *extraout_A1 = *extraout_A0 + 0x80;
  extraout_A1[2] = extraout_A0[2] + 0x80;
  *(byte *)(extraout_A1 + 8) = *(byte *)(extraout_A1 + 8) | 1;
  extraout_A1[1] = unaff_D6w;
  return;
}


