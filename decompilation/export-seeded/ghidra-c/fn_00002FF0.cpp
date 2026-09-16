// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00002FF0


void FUN_00002ff0(void)

{
  short sVar1;
  int iVar2;
  
  iVar2 = 0xff0400;
  sVar1 = 0xc;
  while (((*(byte *)(iVar2 + 0x10) & 1) == 0 || ((*(byte *)(iVar2 + 0xb0) & 0x80) == 0))) {
    iVar2 = iVar2 + 0x100;
    sVar1 = sVar1 + -1;
    if (sVar1 == -1) {
      stop();
      return;
    }
  }
  return;
}


