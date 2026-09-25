// Ghidra headless export
// Entry: 0x0000CFFE
// Name: prepare_title_mode_display
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000C0C2 load_title_font
//   0x00010A82 convert_menu_routine_0x00010a82
//   0x000566D8 convert_intro_routine_0x000566d8
//   0x0005670E refresh_mode_palette_data


void prepare_title_mode_display(void)

{
  convert_intro_routine_0x000566d8();
  refresh_mode_palette_data();
  convert_menu_routine_0x00010a82();
  load_title_font();
  return;
}


