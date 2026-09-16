// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00015A8C


short FUN_00015a8c(void)

{
  short sVar1;
  short sVar2;
  int iVar3;
  
  iVar3 = 0xff0b00;
  sVar2 = 5;
  sVar1 = 9;
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


