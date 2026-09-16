// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00052EA2


void FUN_00052ea2(void)

{
  undefined1 in_D0b;
  short in_D1w;
  byte *in_A0;
  int in_A1;
  
  *(undefined1 *)(in_A1 + in_D1w) = in_D0b;
  *in_A0 = (char)in_D1w + 1U & 0x3f;
  return;
}


