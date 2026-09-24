// Ghidra headless export
// Entry: 0x00019F18
// Name: FUN_00019f18
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00017998 FUN_00017998
//   0x00017C52 render_menu_text_followup
//   0x00017C84 set_hud_selection_value
//   0x0001B706 FUN_0001b706
//   0x000564D6 FUN_000564d6


void FUN_00019f18(void)

{
  set_hud_selection_value();
  render_menu_text_followup();
  if ('\0' < DAT_00fff0d7) {
    FUN_000564d6();
    FUN_00017998();
  }
  FUN_0001b706();
  return;
}


