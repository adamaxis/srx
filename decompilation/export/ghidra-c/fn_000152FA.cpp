// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x000152FA


void FUN_000152fa(void)

{
  undefined2 *in_A1;
  
  *(undefined1 *)((int)in_A1 + 0x41) = 2;
  in_A1[0x2c] = *in_A1;
  in_A1[0x2d] = in_A1[2];
  *(undefined1 *)((int)in_A1 + 0x8b) = 1;
  *(undefined1 *)((int)in_A1 + 0x89) = 0x20;
  return;
}


