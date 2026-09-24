// Ghidra headless export
// Entry: 0x00019B34
// Name: publish_menu_selection_then_text
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00017C52 render_menu_text_followup
//   0x00017C84 set_hud_selection_value


void publish_menu_selection_then_text(void)

{
  set_hud_selection_value();
  render_menu_text_followup();
  return;
}


