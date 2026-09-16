// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00015C58


void FUN_00015c58(void)

{
  ushort unaff_D3w;
  int in_A1;
  
  if (((*(byte *)(in_A1 + 0x10) & 1) != 0) && ((*(byte *)(in_A1 + 0x10) & 0x40) == 0)) {
    FUN_00014d58();
    if (unaff_D3w < 0x200) {
      return;
    }
  }
  return;
}


