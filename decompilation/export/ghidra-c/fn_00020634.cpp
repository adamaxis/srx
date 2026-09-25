// Ghidra headless export
// Entry: 0x00020634
// Name: convert_world_routine_0x00020634
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void convert_world_routine_0x00020634(void)

{
  if (DAT_00fffbf8 == '\0') {
    if (DAT_00fffbf9 == '\x04') {
      DAT_00fff0b4 = 2;
      return;
    }
    if (DAT_00fffbf9 == '\x05') {
      DAT_00fff0b4 = 1;
      return;
    }
  }
  else if (DAT_00fffbf8 == '\x03') {
    if (DAT_00fffbf9 == '\x02') {
      DAT_00fff0b4 = 1;
      return;
    }
    if (DAT_00fffbf9 == '\x03') {
      DAT_00fff0b4 = 2;
      return;
    }
    if (DAT_00fffbf9 == '\x06') {
      DAT_00fff0b4 = 0;
      return;
    }
  }
  else if (DAT_00fffbf8 == '\x01') {
    if (DAT_00fffbf9 == '\x02') {
      DAT_00fff0b4 = 4;
      return;
    }
  }
  else if (DAT_00fffbf8 == '\x02') {
    if (DAT_00fffbf9 == '\0') {
      DAT_00fff0b4 = 0;
      return;
    }
  }
  else if (DAT_00fffbf8 == '\x05') {
    DAT_00fff0b4 = 3;
  }
  return;
}


