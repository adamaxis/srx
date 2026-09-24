// Ghidra headless export
// Entry: 0x00017EB4
// Name: calculate_menu_delay_offset
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000D81C average_entity_delay_attributes


void calculate_menu_delay_offset(void)

{
  char cVar1;
  char extraout_D1b;
  
  cVar1 = average_entity_delay_attributes();
  DAT_00fff0a7 = '\x1d' - (extraout_D1b + cVar1);
  return;
}


