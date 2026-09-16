// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0001CCDC


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0001ccdc(void)

{
  int extraout_A0;
  
  if (DAT_00ff15a4 == '\0') {
    return;
  }
  FUN_0001d048();
  FUN_0001d0de();
  FUN_00001378();
  if (*(char *)(extraout_A0 + 6) == '\0') {
    _DAT_00ffe0d6 = 0;
    FUN_00019e32();
  }
  return;
}


