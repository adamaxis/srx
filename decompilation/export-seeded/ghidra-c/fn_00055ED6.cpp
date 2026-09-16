// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00055ED6


void FUN_00055ed6(void)

{
  short sVar1;
  int in_A0;
  
  sVar1 = 7;
  do {
    if (*(char *)(in_A0 + 0x66) == '(') {
      return;
    }
    in_A0 = in_A0 + 1;
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  return;
}


