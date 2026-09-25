// Ghidra headless export
// Entry: 0x0001A09C
// Name: refresh_menu_selection_after_clear
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00017C52 render_menu_text_followup
//   0x00017C84 set_hud_selection_value
//   0x0001A154 clear_selected_menu_record


void refresh_menu_selection_after_clear(void)

{
  if ('\0' < DAT_00fff0aa) {
    clear_selected_menu_record();
  }
  set_hud_selection_value();
  render_menu_text_followup();
  return;
}


