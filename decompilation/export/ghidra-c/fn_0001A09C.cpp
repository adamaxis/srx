// Ghidra headless export
// Entry: 0x0001A09C
// Name: FUN_0001a09c
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00017C52 render_menu_text_followup
//   0x00017C84 set_hud_selection_value
//   0x0001A154 FUN_0001a154


void FUN_0001a09c(void)

{
  if ('\0' < DAT_00fff0aa) {
    FUN_0001a154();
  }
  set_hud_selection_value();
  render_menu_text_followup();
  return;
}


