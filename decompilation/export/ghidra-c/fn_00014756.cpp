// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00014756


void FUN_00014756(void)

{
  int in_A0;
  
  *(byte *)(in_A0 + 0xb0) = *(byte *)(in_A0 + 0xb0) & 0xfd;
  *(byte *)(in_A0 + 0xb0) = *(byte *)(in_A0 + 0xb0) & 0xf7;
  return;
}


