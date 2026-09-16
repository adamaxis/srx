// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00013CD4


void FUN_00013cd4(void)

{
  short unaff_D2w;
  int in_A0;
  int extraout_A0;
  
  if (*(char *)(in_A0 + 0x9c) != '\x01') {
    FUN_00010660();
    if ((unaff_D2w < 0) || ((byte)unaff_D2w < *(byte *)(extraout_A0 + 0x9c))) {
      return;
    }
  }
  return;
}


