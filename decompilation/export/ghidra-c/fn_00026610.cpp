// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00026610


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00026610(void)

{
  short extraout_D1w;
  short sVar1;
  short sVar2;
  
  _DAT_00c00004 = 0x8f80;
  if ((&DAT_00ff1100)[(short)(_DAT_00fff118 * 0x14)] == '\0') {
    sVar2 = 7;
  }
  else {
    sVar2 = 9;
  }
  sVar1 = 0x393;
  do {
    FUN_00000e9c();
    do {
      _DAT_00c00000 = sVar1;
      sVar1 = _DAT_00c00000 + 1;
      sVar2 = sVar2 + -1;
    } while (sVar2 != -1);
    if ((&DAT_00ff1100)[(short)(_DAT_00fff118 * 0x14)] == '\0') {
      sVar2 = 7;
    }
    else {
      sVar2 = 9;
    }
  } while (extraout_D1w != 0);
  _DAT_00c00004 = 0x8f02;
  return;
}


