// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0001DED8


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0001ded8(void)

{
  short sVar1;
  int iVar2;
  int extraout_A0;
  
  iVar2 = 0xff0400;
  sVar1 = 6;
  _DAT_00fffc3c = 0;
  do {
    if ((*(byte *)(iVar2 + 0xb0) & 0x80) != 0) {
      FUN_0001e514();
      iVar2 = extraout_A0;
    }
    iVar2 = iVar2 + 0x100;
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  return;
}


