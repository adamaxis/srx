// Ghidra headless export
// Entry: 0x00058CDE
// Name: convert_intro_routine_0x00058cde
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000E34 reset_vdp_upload_state
//   0x00058B5E convert_intro_routine_0x00058b5e
//   0x00058BB6 convert_intro_routine_0x00058bb6
//   0x00058D32 convert_intro_routine_0x00058d32


void convert_intro_routine_0x00058cde(void)

{
  char in_D0b;
  
  DAT_00fff0d1 = in_D0b + DAT_00fff0d1;
  if (DAT_00fff0d1 < '\0') {
    DAT_00fff0d1 = 0;
    return;
  }
  if (-1 < (char)(DAT_00fff0d1 + -2)) {
    DAT_00fff0d1 = 1;
    return;
  }
  DAT_00fff0d0 = 0;
  if (DAT_00fff0d1 != '\0') {
    convert_intro_routine_0x00058b5e();
    convert_intro_routine_0x00058d32();
    return;
  }
  reset_vdp_upload_state();
  convert_intro_routine_0x00058b5e();
  convert_intro_routine_0x00058bb6();
  return;
}


