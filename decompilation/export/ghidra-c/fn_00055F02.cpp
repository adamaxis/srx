// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00055F02


void FUN_00055f02(void)

{
  short sVar1;
  int in_A0;
  
  sVar1 = 6;
  do {
    if (*(char *)(in_A0 + 0x66 + (int)sVar1) == '\x18') {
      return;
    }
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  return;
}


