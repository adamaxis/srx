// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x000105E6


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_000105e6(void)

{
  char cVar1;
  bool bVar2;
  char cVar3;
  
  cVar1 = _DAT_00fff096 < 0;
  bVar2 = _DAT_00fff096 == 0;
  cVar3 = '\0';
  FUN_00010660();
  if (bVar2 || cVar3 != cVar1) {
                    /* WARNING: Subroutine does not return */
    FUN_0000c0da();
  }
  FUN_00001232();
  return;
}


