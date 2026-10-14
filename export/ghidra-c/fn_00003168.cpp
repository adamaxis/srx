// Ghidra headless export
// Entry: 0x00003168
// Name: toggle_field_scene_variant
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void toggle_field_scene_variant(void)

{
  if (DAT_00fffbf9 == '\x10') {
    if (DAT_00ffddc4 == 0) {
      if (DAT_00ffddc6 < 0x1400) {
        DAT_00ffddc4 = 3;
        DAT_00ffe114 = &UNK_00003222;
      }
      else {
        DAT_00ffddc4 = 5;
        DAT_00ffe114 = &UNK_00003212;
      }
    }
    else {
      if (DAT_00ffddc4 == 5) {
        DAT_00ffe114 = &UNK_000031f2;
      }
      else {
        DAT_00ffe114 = &UNK_00003202;
      }
      DAT_00ffddc4 = 0;
    }
    return;
  }
  return;
}


