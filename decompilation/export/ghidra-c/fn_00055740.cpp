// Ghidra headless export
// Entry: 0x00055740
// Name: convert_intro_routine_0x00055740
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void convert_intro_routine_0x00055740(void)

{
  int in_A0;
  int in_A1;
  
  if ((((*(char *)(in_A1 + 0x9b) != '\x16') && ((*(ushort *)(in_A0 + 0x90) & 2) == 0)) &&
      (*(char *)(in_A0 + 0x9a) != '\x04')) &&
     ((*(char *)(in_A0 + 0x9a) != '\x02' && ((*(byte *)(in_A1 + 0x11) & 0x80) != 0)))) {
    return;
  }
  return;
}


