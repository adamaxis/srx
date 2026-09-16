// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x000104F6


void FUN_000104f6(void)

{
  short sVar1;
  char unaff_D5b;
  int in_A0;
  
  sVar1 = 6;
  do {
    if (unaff_D5b == *(char *)(in_A0 + 0x66 + (int)sVar1)) {
      return;
    }
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  return;
}


