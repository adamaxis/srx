// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00019D66


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00019d66(void)

{
  if ((char)(&DAT_00fffb30)[_DAT_00fff102] < '\x01') {
    (&DAT_00fffb30)[_DAT_00fff102] = 0;
    FUN_00017c84();
    FUN_00017c52();
    FUN_00017e04();
    FUN_0001b706();
    return;
  }
  FUN_00017c84();
  FUN_00017c52();
  FUN_00019dac();
  FUN_00017998();
  FUN_00017a5e();
  return;
}


