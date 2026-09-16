// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00016760


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00016760(void)

{
  int iVar1;
  short sVar2;
  
  sVar2 = FUN_000166d0();
  iVar1 = (int)_DAT_00ffddea;
  if (*(short *)(&DAT_00ff2700 + iVar1 + sVar2) == 0xb) {
    return;
  }
  *(char *)(_DAT_00fff0c8 + 0xffe179) = *(char *)(_DAT_00fff0c8 + 0xffe179) + '\x01';
  *(short *)(&DAT_00ff2700 + iVar1 + sVar2) = 0xb;
  _DAT_00ffe100 = 1;
  return;
}


