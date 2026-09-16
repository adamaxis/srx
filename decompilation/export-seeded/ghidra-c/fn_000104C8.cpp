// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x000104C8


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_000104c8(void)

{
  short sVar1;
  
  sVar1 = 6;
  do {
    if ((char)(*(char *)(_DAT_00fff10e +
                        (short)((short)CONCAT31((int3)((uint)DAT_00fff0d1 * 6 >> 8),
                                                DAT_00fff0d0 + (char)((uint)DAT_00fff0d1 * 6)) * 2 +
                               2)) + '\x19') == *(char *)(_DAT_00fffc04 + 0x66 + (int)sVar1)) {
      return;
    }
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  return;
}


