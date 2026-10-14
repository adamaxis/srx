// Ghidra headless export
// Entry: 0x00017EB4
// Name: calculate_menu_delay_offset
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000D81C average_entity_delay_attributes
// Behavior annotation from the ROM disassembly:
//   D0 starts at six, adds four times FFFBBE+0x07, calls 0xD81C for the
//   active record's wrapped average at A0+0x78/+0x7B, adds that result, then
//   stores the low byte of 0x1D-D0 at FFF0A7. The typed conversion exposes
//   those three source inputs directly.


void calculate_menu_delay_offset(void)

{
  char cVar1;
  char extraout_D1b;
  
  cVar1 = average_entity_delay_attributes();
  DAT_00fff0a7 = '\x1d' - (extraout_D1b + cVar1);
  return;
}


