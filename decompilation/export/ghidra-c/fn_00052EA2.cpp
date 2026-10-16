// Ghidra headless export
// Entry: 0x00052EA2
// Name: append_audio_command
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void append_audio_command(void)

{
  undefined1 in_D0b;
  short in_D1w;
  byte *in_A0;
  int in_A1;
  
  *(undefined1 *)(in_A1 + in_D1w) = in_D0b;
  *in_A0 = (char)in_D1w + 1U & 0x3f;
  return;
}


