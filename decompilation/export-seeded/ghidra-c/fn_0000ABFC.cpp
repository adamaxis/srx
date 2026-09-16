// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0000ABFC


void FUN_0000abfc(void)

{
  short sVar1;
  short extraout_D1w;
  short sVar2;
  int extraout_A0;
  
  sVar1 = FUN_0000d7ea();
  sVar2 = extraout_D1w * -4 + 0x6a;
  if (sVar1 != 0) {
    if (sVar1 == 2) {
      sVar2 = sVar2 * 2;
    }
    else {
      sVar2 = sVar2 >> 1;
    }
  }
  if (sVar2 < 0xf) {
    sVar2 = 0xf;
  }
  *(short *)(extraout_A0 + 0xdc) = sVar2;
  return;
}


