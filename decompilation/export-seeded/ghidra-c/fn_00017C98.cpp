// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00017C98


void FUN_00017c98(void)

{
  char in_D0b;
  short sVar1;
  short sVar2;
  
  sVar1 = 0;
  sVar2 = 4;
  do {
    if (in_D0b == (&DAT_00fffbd8)[sVar1]) {
      return;
    }
    sVar1 = sVar1 + 1;
    sVar2 = sVar2 + -1;
  } while (sVar2 != -1);
  return;
}


