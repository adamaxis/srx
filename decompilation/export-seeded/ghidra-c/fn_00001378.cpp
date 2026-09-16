// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00001378


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00001378(void)

{
  short in_D1w;
  short unaff_D2w;
  char unaff_D5b;
  ushort unaff_D6w;
  short *unaff_A2;
  ushort *puVar1;
  short *unaff_A3;
  
  while( true ) {
    if (*unaff_A2 == -0x8000) break;
    unaff_A3[3] = in_D1w + *unaff_A2;
    *unaff_A3 = unaff_D2w + unaff_A2[1];
    puVar1 = (ushort *)(unaff_A2 + 3);
    *(char *)(unaff_A3 + 1) = (char)unaff_A2[2];
    *(char *)((int)unaff_A3 + 3) = unaff_D5b;
    unaff_A2 = unaff_A2 + 4;
    unaff_A3[2] = unaff_D6w | *puVar1;
    unaff_D5b = unaff_D5b + '\x01';
    _DAT_00ffdd02 = _DAT_00ffdd02 + 1;
    unaff_A3 = unaff_A3 + 4;
    _DAT_00ffdd18 = unaff_A3;
    _DAT_00ffdd1c = _DAT_00ffdd02;
  }
  return;
}


