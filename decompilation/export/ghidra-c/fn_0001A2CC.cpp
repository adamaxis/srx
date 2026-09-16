// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0001A2CC


short FUN_0001a2cc(void)

{
  short sVar1;
  short sVar2;
  char *pcVar3;
  
  sVar2 = 0x11;
  sVar1 = 0;
  pcVar3 = &DAT_00ffd86c;
  do {
    if (*pcVar3 == '\0') {
      *pcVar3 = '\x01';
      return sVar1;
    }
    sVar1 = sVar1 + 1;
    sVar2 = sVar2 + -1;
    pcVar3 = pcVar3 + 1;
  } while (sVar2 != -1);
  return sVar1;
}


