// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00012A8C


void FUN_00012a8c(void)

{
  short sVar1;
  
  sVar1 = FUN_00012a54();
  if (-1 < *(int *)((int)&PTR_DAT_000d455e + (int)(short)((sVar1 + -1) * 0x30))) {
    FUN_00001232();
  }
                    /* WARNING: Subroutine does not return */
  FUN_0000c0da();
}


