// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0001055E


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0001055e(void)

{
  short in_D0w;
  char cVar1;
  int extraout_A0;
  
  if (in_D0w == 0) {
    FUN_00055f02();
    FUN_00055f20();
    FUN_000104c8();
    cVar1 = '\0';
  }
  else {
    FUN_0001047e();
    cVar1 = *(char *)(extraout_A0 + 0x7c);
  }
  while (FUN_00001232(), cVar1 != '\0') {
    cVar1 = '\0';
  }
  return;
}


