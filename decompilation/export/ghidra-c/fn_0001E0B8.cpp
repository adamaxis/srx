// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0001E0B8


void FUN_0001e0b8(void)

{
  short sVar1;
  int iVar2;
  
  iVar2 = 0xff0400;
  sVar1 = 6;
  do {
    if ((*(byte *)(iVar2 + 0x9e) & 2) != 0) {
      *(byte *)(iVar2 + 0x9e) = *(byte *)(iVar2 + 0x9e) | 4;
    }
    iVar2 = iVar2 + 0x100;
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  return;
}


