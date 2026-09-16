// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00000E34


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined2 FUN_00000e34(void)

{
  undefined2 in_D0w;
  char in_XF;
  char in_NF;
  char in_ZF;
  char in_VF;
  byte in_CF;
  
  FUN_00000e9c(in_XF << 4 | in_NF << 3 | in_ZF << 2 | in_VF << 1 | in_CF);
  _DAT_00c00000 = 0;
  _DAT_00ffdd18 = &DAT_00ffd986;
  _DAT_00ffdd1c = 1;
  _DAT_00ffd97e = 0;
  return in_D0w;
}


