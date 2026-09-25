// Ghidra headless export
// Entry: 0x00058C54
// Name: convert_intro_routine_0x00058c54
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00058B5E convert_intro_routine_0x00058b5e
//   0x00058BB6 convert_intro_routine_0x00058bb6
//   0x00058D32 convert_intro_routine_0x00058d32


void convert_intro_routine_0x00058c54(void)

{
  char in_D0b;
  char cVar1;
  
  if (DAT_00fff0d1 == '\0') {
    if (DAT_00fff0b7 == '\0') {
      cVar1 = '\x03';
    }
    else {
      cVar1 = '\x05';
    }
    if ((char)(in_D0b + DAT_00fff0d0) < '\0') {
      DAT_00fff0d0 = 0;
      return;
    }
    if ((char)(cVar1 - (in_D0b + DAT_00fff0d0)) < '\0') {
      DAT_00fff0d0 = cVar1;
      return;
    }
  }
  else {
    if ((char)(in_D0b + DAT_00fff0d0) < '\0') {
      DAT_00fff0d0 = 0;
      return;
    }
    if (-1 < (char)(in_D0b + DAT_00fff0d0 + -8)) {
      DAT_00fff0d0 = 7;
      return;
    }
  }
  DAT_00fff0d0 = in_D0b + DAT_00fff0d0;
  if (DAT_00fff0d1 != '\0') {
    convert_intro_routine_0x00058d32();
    return;
  }
  convert_intro_routine_0x00058b5e();
  convert_intro_routine_0x00058bb6();
  return;
}


