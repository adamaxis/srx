// Ghidra headless export
// Entry: 0x00019DAC
// Name: merge_menu_pending_flags
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void merge_menu_pending_flags(void)

{
  *(byte *)((int)&DAT_00fffb30 + (int)DAT_00fff102) =
       *(byte *)((int)&DAT_00ff1104 + (short)(DAT_00fff118 * 0x14) + 1) |
       *(byte *)((int)&DAT_00fffb30 + (int)DAT_00fff102);
  FUN_000566d8();
  return;
}


