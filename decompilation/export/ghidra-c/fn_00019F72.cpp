// Ghidra headless export
// Entry: 0x00019F72
// Name: finalize_menu_slot_code_five
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void finalize_menu_slot_code_five(void)

{
  short extraout_D1w;
  short sVar1;
  
  sVar1 = 4;
  do {
    if ((&DAT_00fffbd8)[sVar1] == '\x05') {
      clear_menu_slot_visual();
      (&DAT_00fff0e8)[sVar1] = 0;
      *(short *)((int)&DAT_00fff0f2 + (int)(short)(sVar1 << 1)) = extraout_D1w * 10;
      refresh_menu_number_or_grid();
      clear_menu_status_at();
      return;
    }
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  return;
}


