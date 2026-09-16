// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0001A342


short FUN_0001a342(void)

{
  short sVar1;
  short sVar2;
  int iVar3;
  
  iVar3 = 0xff0400;
  sVar2 = 6;
  sVar1 = 3;
  do {
    if ((*(byte *)(iVar3 + 0x10) & 1) == 0) {
      return sVar1;
    }
    iVar3 = iVar3 + 0x100;
    sVar1 = sVar1 + 1;
    sVar2 = sVar2 + -1;
  } while (sVar2 != -1);
  return sVar1;
}


