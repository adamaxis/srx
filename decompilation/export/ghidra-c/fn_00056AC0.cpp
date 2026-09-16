// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00056AC0


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00056ac0(void)

{
  DAT_00fff0b1 = 0;
  _DAT_00c00004 = 0x9100;
  FUN_00010bc4();
  FUN_00011048();
  FUN_0005670e();
  if (DAT_00fff0b7 == '\x01') {
    FUN_00000f5c();
  }
  if (DAT_00fff0b7 == '\x02') {
    FUN_000566e2();
    FUN_000566f6();
    FUN_00001530();
  }
  FUN_0000c0c2();
                    /* WARNING: Subroutine does not return */
  FUN_0000c0da();
}


