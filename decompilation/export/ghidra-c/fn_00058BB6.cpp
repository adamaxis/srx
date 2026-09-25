// Ghidra headless export
// Entry: 0x00058BB6
// Name: convert_intro_routine_0x00058bb6
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001232 write_vdp_grid
//   0x0000C0DA render_encoded_title_text


void convert_intro_routine_0x00058bb6(void)

{
  if (((DAT_00fff0b7 < '\0') || (DAT_00fff0b7 == '\0')) || (DAT_00fff0b7 != '\x01')) {
    write_vdp_grid();
    render_encoded_title_text();
  }
  return;
}


