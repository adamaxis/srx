// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00016E5C


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

short FUN_00016e5c(void)

{
  short *psVar1;
  short *psVar2;
  short *extraout_A1;
  char *pcVar3;
  char *pcVar4;
  
  psVar1 = *(short **)(_DAT_00ffde1e + 8);
  pcVar3 = &DAT_00ffb700;
  while (psVar2 = psVar1, *psVar2 != 0) {
    pcVar4 = pcVar3 + 1;
    if (*pcVar3 == '\0') {
      if (psVar2[2] < 0) {
        pcVar4 = pcVar3;
      }
      FUN_00016ea0();
      psVar1 = extraout_A1;
      pcVar3 = pcVar4;
    }
    else {
      psVar1 = psVar2 + 3;
      pcVar3 = pcVar4;
      if (psVar2[2] < 0) {
        psVar1 = psVar2 + 6;
      }
    }
  }
  return *psVar2;
}


