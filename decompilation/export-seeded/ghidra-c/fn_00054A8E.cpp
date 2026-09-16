// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00054A8E


void FUN_00054a8e(void)

{
  char cVar1;
  
  cVar1 = FUN_00000d3a();
  if (DAT_00fff0a8 <= cVar1) {
    return;
  }
  DAT_00fff0a8 = 0;
  FUN_0001989a();
  if (DAT_00fff0b1 == '\0') {
    FUN_000549f6();
                    /* WARNING: Subroutine does not return */
    FUN_0000c0da();
  }
  FUN_000549e4();
  return;
}


