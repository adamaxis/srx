// Ghidra headless export
// Entry: 0x00058AA4
// Name: convert_intro_routine_0x00058aa4
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000C0DA render_encoded_title_text
//   0x00011190 transfer_menu_glyph_triple


void convert_intro_routine_0x00058aa4(void)

{
  short sVar1;
  
  sVar1 = 7;
  do {
    transfer_menu_glyph_triple();
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  if (DAT_00fff0b8 == '\x04') {
    render_encoded_title_text();
  }
  return;
}


