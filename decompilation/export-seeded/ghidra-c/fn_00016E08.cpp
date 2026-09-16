// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00016E08


void FUN_00016e08(void)

{
  short sVar1;
  int iVar2;
  
  iVar2 = 0xff164c;
  sVar1 = 9;
  do {
    if ((*(byte *)(iVar2 + 0x10) & 1) == 0) {
      return;
    }
    iVar2 = iVar2 + 0x12;
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  return;
}


