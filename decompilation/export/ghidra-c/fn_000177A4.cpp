// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x000177A4


void FUN_000177a4(void)

{
  short sVar1;
  
  sVar1 = 4;
  do {
    (&DAT_00fff0ed)[sVar1] = 6;
    *(undefined1 *)(sVar1 + 0xfff0e8) = 1;
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  return;
}


