// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0000481E


void FUN_0000481e(void)

{
  short sVar1;
  int iVar2;
  int extraout_A0;
  int extraout_A0_00;
  
  iVar2 = 0xff164c;
  sVar1 = 9;
  do {
    if (((*(byte *)(iVar2 + 0x10) & 1) != 0) &&
       (FUN_00005a62(), iVar2 = extraout_A0, *(short *)(extraout_A0 + 0xc) != 0)) {
      FUN_00001378();
      iVar2 = extraout_A0_00;
    }
    iVar2 = iVar2 + 0x12;
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  return;
}


