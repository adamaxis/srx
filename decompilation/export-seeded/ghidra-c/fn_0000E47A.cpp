// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0000E47A


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0000e47a(void)

{
  FUN_000530de();
  FUN_00000c24();
  FUN_000181f8();
  if (DAT_00fff0b1 == '\0') {
    FUN_000173f2();
    DAT_00fff0dc = 0xff;
    if (*(char *)(_DAT_00fffc04 + 0x4c) == '\0') {
      FUN_0000b330();
    }
  }
  DAT_00fff0b1 = 0;
  return;
}


