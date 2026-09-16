// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00056632


int FUN_00056632(void)

{
  int iVar1;
  short unaff_D4w;
  
  if ((short)(unaff_D4w + -2) < 0) {
    unaff_D4w = 2;
  }
  if ((ushort)(7U - unaff_D4w) == 0 || 7 < unaff_D4w) {
    iVar1 = 10;
  }
  else {
    iVar1 = (uint)(ushort)(7U - unaff_D4w) * 0x14;
  }
  return iVar1;
}


