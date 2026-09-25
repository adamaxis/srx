// Ghidra headless export
// Entry: 0x00058B5E
// Name: convert_intro_routine_0x00058b5e
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001232 write_vdp_grid
//   0x00057C46 convert_intro_routine_0x00057c46


void convert_intro_routine_0x00058b5e(void)

{
  convert_intro_routine_0x00057c46();
  if (((DAT_00fff0b7 < '\0') || (DAT_00fff0b7 == '\0')) || (DAT_00fff0b7 != '\x01')) {
    write_vdp_grid();
  }
  return;
}


