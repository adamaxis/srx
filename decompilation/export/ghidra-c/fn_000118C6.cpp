// Ghidra headless export
// Entry: 0x000118C6
// Name: cycle_inventory_action_selector
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void cycle_inventory_action_selector(void)

{
  char in_D0b;
  
  DAT_00fff0d0 = in_D0b + DAT_00fff0d0;
  if ((char)(DAT_00fff0d0 + -2) < '\0') {
    DAT_00fff0d0 = 5;
    DAT_00fff112 = 0xff;
    return;
  }
  if (DAT_00fff0d0 == '\x06') {
    DAT_00fff0d0 = '\x02';
  }
  return;
}


