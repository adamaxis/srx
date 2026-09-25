// Ghidra headless export
// Entry: 0x0005878E
// Name: convert_intro_routine_0x0005878e
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000C0DA render_encoded_title_text


void convert_intro_routine_0x0005878e(void)

{
  short in_D0w;
  
  if ((((-1 < (short)(in_D0w + -0x19)) && (-1 < (short)(in_D0w + -0x25))) &&
      (-1 < (short)(in_D0w + -0x28))) && ((short)(in_D0w + -0x2b) < 0)) {
    return;
  }
  render_encoded_title_text();
  return;
}


