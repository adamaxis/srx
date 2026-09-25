// Ghidra headless export
// Entry: 0x000549F6
// Name: select_display_status
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void select_display_status(void)

{
  if (DAT_00fff0a0 == '\0') {
    DAT_00ffd8fc = 0xcc;
  }
  else if (DAT_00fff0a0 < '\x01') {
    DAT_00ffd8fc = 0xc0;
  }
  else {
    DAT_00ffd8fc = 0xc;
  }
  DAT_00ffef14 = DAT_00ffef14 | 1;
  return;
}


