// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0005B32C


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined8 FUN_0005b32c(void)

{
  undefined4 in_D0;
  undefined4 in_D1;
  short extraout_D1w;
  short unaff_D7w;
  short sVar1;
  
  do {
    FUN_00000e9c();
    sVar1 = unaff_D7w;
    do {
      _DAT_00c00000 = 0x7ff;
      sVar1 = sVar1 + -1;
    } while (sVar1 != -1);
  } while (extraout_D1w != 0);
  return CONCAT44(in_D0,in_D1);
}


