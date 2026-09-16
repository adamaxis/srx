// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00003648


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00003648(void)

{
  short extraout_D1w;
  ushort unaff_D2w;
  ushort unaff_D5w;
  ushort uVar1;
  short unaff_D7w;
  short sVar2;
  
  uVar1 = unaff_D2w | unaff_D5w;
  do {
    FUN_00000e9c();
    sVar2 = unaff_D7w;
    do {
      _DAT_00c00000 = uVar1;
      uVar1 = _DAT_00c00000 + 1;
      sVar2 = sVar2 + -1;
    } while (sVar2 != -1);
  } while (extraout_D1w != 0);
  return;
}


