// Ghidra headless export
// Entry: 0x00019EA6
// Name: run_menu_selection_variant
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00017C52 render_menu_text_followup
//   0x00017C84 set_hud_selection_value
//   0x0001B706 prepare_menu_offset_once
//   0x00056482 convert_intro_routine_0x00056482


void run_menu_selection_variant(void)

{
  set_hud_selection_value();
  render_menu_text_followup();
  if ('\0' < DAT_00fff0d6) {
    convert_intro_routine_0x00056482();
  }
  prepare_menu_offset_once();
  return;
}


