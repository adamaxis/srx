// Ghidra headless export
// Entry: 0x00058AA4
// Name: FUN_00058aa4
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000C0DA render_title_text_source
//   0x00011190 FUN_00011190


void FUN_00058aa4(void)

{
  short sVar1;
  
  sVar1 = 7;
  do {
    FUN_00011190();
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  if (DAT_00fff0b8 == '\x04') {
    render_title_text_source();
  }
  return;
}


