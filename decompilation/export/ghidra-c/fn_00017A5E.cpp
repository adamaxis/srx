// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00017A5E


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00017a5e(void)

{
  char *pcVar1;
  byte bVar2;
  char cVar3;
  char cVar4;
  bool bVar5;
  char cVar6;
  
  bVar2 = *(byte *)((short)(_DAT_00fff118 * 0x14) + 0xff1113);
  if ((char)bVar2 < '\0') {
    FUN_00019dac();
    (&DAT_00fffb30)[_DAT_00fff102] = (&DAT_00fffb30)[_DAT_00fff102] | 0x40;
  }
  else {
    pcVar1 = (char *)((short)(_DAT_00fff118 * 0x14) + 0xff110f);
    cVar6 = '\0';
    cVar4 = *pcVar1 < '\0';
    cVar3 = *pcVar1;
    bVar5 = cVar3 == '\0';
    if (('\0' < cVar3) && (FUN_00017c3c(), !bVar5 && cVar6 == cVar4)) {
      if (DAT_00ffdf84 != '\0') {
        FUN_00017e04();
      }
      if ((char)(&DAT_000e31a7)[(uint)bVar2 * 2] < '\0') {
        return;
      }
                    /* WARNING: Subroutine does not return */
      FUN_0000c0da();
    }
  }
                    /* WARNING: Subroutine does not return */
  FUN_0000c0da();
}


