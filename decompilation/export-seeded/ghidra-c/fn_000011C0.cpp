// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x000011C0


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined8 FUN_000011c0(void)

{
  undefined4 in_D0;
  undefined4 in_D1;
  short extraout_D1w;
  ushort unaff_D2w;
  short unaff_D7w;
  short sVar1;
  ushort *extraout_A1;
  
  do {
    FUN_00000e9c();
    sVar1 = unaff_D7w;
    do {
      _DAT_00c00000 = unaff_D2w | *extraout_A1;
      sVar1 = sVar1 + -1;
    } while (sVar1 != -1);
  } while (extraout_D1w != 0);
  return CONCAT44(in_D0,in_D1);
}


