// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00052E96


void FUN_00052e96(void)

{
  byte bVar1;
  undefined1 in_D0b;
  short in_D1w;
  byte *in_A0;
  int in_A1;
  
  *(undefined1 *)(in_A1 + in_D1w) = 0xff;
  bVar1 = (char)in_D1w + 1;
  *(undefined1 *)(in_A1 + (short)(CONCAT11((char)((ushort)in_D1w >> 8),bVar1) & 0xff3f)) = in_D0b;
  *in_A0 = (bVar1 & 0x3f) + 1 & 0x3f;
  return;
}


