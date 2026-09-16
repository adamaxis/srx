// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00006148


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00006148(void)

{
  short sVar1;
  
  sVar1 = _DAT_00ffdf2a + -1;
  do {
    FUN_00000f5c();
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  _DAT_00ffdf2a = 0;
  return;
}


