// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x000019F2


void FUN_000019f2(void)

{
  short sVar1;
  
  FUN_00052dc2();
  DAT_00a10009 = 0x40;
  sVar1 = 10;
  do {
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  DAT_00a10003 = 0x40;
  sVar1 = 10;
  do {
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  DAT_00ffef0e = 0xff;
  FUN_00052dd6();
  return;
}


