// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0001A322


void FUN_0001a322(void)

{
  short sVar1;
  int iVar2;
  
  DAT_00ffe177 = 0;
  iVar2 = 0xff0400;
  sVar1 = 0xc;
  do {
    *(byte *)(iVar2 + 0x10) = *(byte *)(iVar2 + 0x10) & 0xfe;
    iVar2 = iVar2 + 0x100;
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  return;
}


