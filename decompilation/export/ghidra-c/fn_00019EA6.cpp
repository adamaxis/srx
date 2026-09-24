// Ghidra headless export
// Entry: 0x00019EA6
// Name: FUN_00019ea6
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00017C52 render_menu_text_followup
//   0x00017C84 set_hud_selection_value
//   0x0001B706 prepare_menu_offset_once
//   0x00056482 FUN_00056482


void FUN_00019ea6(void)

{
  set_hud_selection_value();
  render_menu_text_followup();
  if ('\0' < DAT_00fff0d6) {
    FUN_00056482();
  }
  prepare_menu_offset_once();
  return;
}


