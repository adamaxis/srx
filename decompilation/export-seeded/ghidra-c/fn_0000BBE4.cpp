// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0000BBE4


void FUN_0000bbe4(void)

{
  undefined2 unaff_D5w;
  undefined2 unaff_D6w;
  undefined2 *in_A0;
  
  *(byte *)(in_A0 + 0x58) = *(byte *)(in_A0 + 0x58) & 0xfe;
  *in_A0 = unaff_D5w;
  in_A0[2] = unaff_D6w;
  return;
}


