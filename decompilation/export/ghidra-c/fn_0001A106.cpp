// Ghidra headless export
// Entry: 0x0001A106
// Name: FUN_0001a106
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00017C52 render_menu_text_followup
//   0x00017C84 set_hud_selection_value
//   0x0001A154 clear_selected_menu_record
//   0x0001B706 prepare_menu_offset_once


void FUN_0001a106(void)

{
  if (DAT_00fff0aa < '\x01') {
    prepare_menu_offset_once();
  }
  else {
    clear_selected_menu_record();
  }
  set_hud_selection_value();
  render_menu_text_followup();
  return;
}


