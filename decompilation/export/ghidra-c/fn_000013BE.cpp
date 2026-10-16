// Ghidra headless export
// Entry: 0x000013BE
// Name: build_title_sprite_records_with_attribute_offset
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void build_title_sprite_records_with_attribute_offset(void)

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
    DAT_00ffdd02 = DAT_00ffdd02 + 1;
    unaff_A3 = unaff_A3 + 4;
    _DAT_00ffdd18 = unaff_A3;
    _DAT_00ffdd1c = DAT_00ffdd02;
  }
  return;
}


