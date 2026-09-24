// Ghidra headless export
// Entry: 0x00017C84
// Name: set_hud_selection_value
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00017C98 find_hud_selection_index


void set_hud_selection_value(void)

{
  undefined1 extraout_D1b;
  short unaff_D4w;
  int unaff_A5;
  
  find_hud_selection_index();
  *(undefined1 *)(unaff_A5 + unaff_D4w) = extraout_D1b;
  return;
}


