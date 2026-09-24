// Ghidra headless export
// Entry: 0x000104C8
// Name: find_matching_menu_metric
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void find_matching_menu_metric(void)

{
  short sVar1;
  
  sVar1 = 6;
  do {
    if ((char)(*(char *)(DAT_00fff10e +
                        (short)((short)CONCAT31((int3)((uint)DAT_00fff0d1 * 6 >> 8),
                                                DAT_00fff0d0 + (char)((uint)DAT_00fff0d1 * 6)) * 2 +
                               2)) + '\x19') == *(char *)(DAT_00fffc04 + 0x66 + (int)sVar1)) {
      return;
    }
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  return;
}


