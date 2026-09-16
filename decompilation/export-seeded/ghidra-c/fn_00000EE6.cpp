// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00000EE6


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00000ee6(void)

{
  if (0x4f < _DAT_00ffdd02) {
    DAT_00ffdbf9 = 0;
  }
  if (_DAT_00ffdd02 != 0xffff) {
    FUN_00000f98();
    return;
  }
  FUN_00000e34();
  return;
}


