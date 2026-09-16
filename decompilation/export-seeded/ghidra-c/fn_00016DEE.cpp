// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00016DEE


void FUN_00016dee(void)

{
  short sVar1;
  int iVar2;
  
  iVar2 = 0xff164c;
  sVar1 = 9;
  do {
    *(byte *)(iVar2 + 0x10) = *(byte *)(iVar2 + 0x10) & 0xfe;
    iVar2 = iVar2 + 0x12;
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  return;
}


