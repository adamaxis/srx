// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x000013BE


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_000013be(void)

{
  short in_D1w;
  short unaff_D2w;
  short unaff_D4w;
  char unaff_D5b;
  ushort unaff_D6w;
  short *unaff_A2;
  short *psVar1;
  short *unaff_A3;
  
  while( true ) {
    if (*unaff_A2 == -0x8000) break;
    unaff_A3[3] = in_D1w + *unaff_A2;
    *unaff_A3 = unaff_D2w + unaff_A2[1];
    psVar1 = unaff_A2 + 3;
    *(char *)(unaff_A3 + 1) = (char)unaff_A2[2];
    *(char *)((int)unaff_A3 + 3) = unaff_D5b;
    unaff_A2 = unaff_A2 + 4;
    unaff_A3[2] = unaff_D6w | unaff_D4w + *psVar1;
    unaff_D5b = unaff_D5b + '\x01';
    _DAT_00ffdd02 = _DAT_00ffdd02 + 1;
    unaff_A3 = unaff_A3 + 4;
    _DAT_00ffdd18 = unaff_A3;
    _DAT_00ffdd1c = _DAT_00ffdd02;
  }
  return;
}


