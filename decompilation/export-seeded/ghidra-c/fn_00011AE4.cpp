// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00011AE4


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00011ae4(void)

{
  char cVar1;
  int iVar2;
  char in_D0b;
  
  iVar2 = _DAT_00fffc04;
  cVar1 = in_D0b + *(char *)(_DAT_00fffc04 + 0x4d);
  *(char *)(_DAT_00fffc04 + 0x4d) = cVar1;
  if (cVar1 < '\0') {
    *(undefined1 *)(iVar2 + 0x4d) = 0;
  }
  else if ('\x04' < *(char *)(iVar2 + 0x4d)) {
    *(undefined1 *)(iVar2 + 0x4d) = 4;
  }
  FUN_00001232();
                    /* WARNING: Subroutine does not return */
  FUN_0000c0da();
}


