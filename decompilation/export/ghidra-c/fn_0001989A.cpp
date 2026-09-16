// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0001989A


void FUN_0001989a(void)

{
  char in_D1b;
  
  DAT_00fff0a0 = in_D1b + DAT_00fff0a0;
  if ((DAT_00fff0a0 != '\0') && (DAT_00fff0a0 != '\x01')) {
    if (DAT_00fff0a0 < '\x02') {
      DAT_00fff0a0 = -1;
    }
    else {
      DAT_00fff0a0 = '\x01';
      if (DAT_00fff0a6 != '\0') {
        FUN_00018776();
        FUN_00026a00();
                    /* WARNING: Subroutine does not return */
        FUN_0000c0da();
      }
    }
  }
  if (-1 < DAT_00fff0a6) {
                    /* WARNING: Subroutine does not return */
    FUN_0000c0da();
  }
  return;
}


