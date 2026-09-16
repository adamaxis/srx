// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0001438C


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0001438c(void)

{
  short sVar1;
  int iVar2;
  int *piVar3;
  int *piVar4;
  
  iVar2 = 0xff0400;
  _DAT_00ffdf94 = 0;
  sVar1 = 6;
  piVar3 = (int *)&DAT_00ffdf96;
  do {
    piVar4 = piVar3;
    if (((((*(byte *)(iVar2 + 0x10) & 1) != 0) && ((*(byte *)(iVar2 + 0x10) & 0x40) == 0)) &&
        ((*(byte *)(iVar2 + 0x10) & 0x10) == 0)) &&
       ((((*(byte *)(iVar2 + 0xb0) & 0x80) == 0 && ((*(byte *)(iVar2 + 0x9e) & 0x20) == 0)) &&
        (((*(byte *)(iVar2 + 0x9e) & 8) == 0 && (*(short *)(iVar2 + 0xc) != 0)))))) {
      _DAT_00ffdf94 = _DAT_00ffdf94 + 1;
      piVar4 = piVar3 + 1;
      *piVar3 = iVar2;
    }
    iVar2 = iVar2 + 0x100;
    sVar1 = sVar1 + -1;
    piVar3 = piVar4;
  } while (sVar1 != -1);
  return;
}


