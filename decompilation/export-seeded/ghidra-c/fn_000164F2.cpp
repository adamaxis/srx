// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x000164F2


void FUN_000164f2(void)

{
  short sVar1;
  short *psVar2;
  
  psVar2 = (short *)&DAT_00ffd740;
  sVar1 = 9;
  do {
    if (*psVar2 == 0) {
      return;
    }
    psVar2 = psVar2 + 0xf;
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  return;
}


