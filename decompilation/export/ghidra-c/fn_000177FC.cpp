// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x000177FC


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_000177fc(void)

{
  short sVar1;
  
  sVar1 = 0x3c;
  do {
    FUN_00000bd2();
    FUN_00000bf2();
    FUN_00017cc8();
    FUN_00017fda();
    _DAT_00ffdd02 = 1;
    FUN_0001c3fa();
    if (DAT_00fff0e1 != '\0') {
      FUN_000268e6();
    }
    _DAT_00ffd980 = 1;
    DAT_00ffef14 = DAT_00ffef14 | 2;
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  FUN_000266e4();
  FUN_0002679e();
  DAT_00fff0d4 = 0;
  FUN_000194ac();
  return;
}


