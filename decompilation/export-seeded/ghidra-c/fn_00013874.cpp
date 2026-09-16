// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00013874


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00013874(void)

{
  DAT_00ffe175 = 0;
  if (_DAT_00ffdfe8 == 0) {
    if ((_DAT_00ffdf88 & 0x3f) == 0xb) {
      FUN_0001438c();
    }
    else if ((_DAT_00ffdf88 & 0x3f) == 0xf) {
      FUN_00014446();
    }
  }
  else {
    FUN_0001438c();
    FUN_00014446();
  }
  return;
}


