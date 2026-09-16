// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00017708


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00017708(void)

{
  short sVar1;
  
  _DAT_00c00004 = 0x8134;
  FUN_00001728();
  sVar1 = 4;
  do {
    FUN_00001232();
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  FUN_00019856();
  if ((DAT_00ffdf84 != '\0') && (sVar1 = FUN_00019bae(), sVar1 == 8)) {
    FUN_00017d72();
    FUN_00017d90();
    FUN_00017f8e();
  }
  return;
}


