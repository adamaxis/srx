// Ghidra headless export
// Entry: 0x00057628
// Name: select_menu_resource_entry
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0005763C select_menu_resource


int select_menu_resource_entry(void)

{
  select_menu_resource();
  return (uint)DAT_00fff0d0 * 2;
}

