// Ghidra headless export
// Entry: 0x00017C84
// Name: FUN_00017c84
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00017C98 find_hud_selection_index


void FUN_00017c84(void)

{
  undefined1 extraout_D1b;
  short unaff_D4w;
  int unaff_A5;
  
  find_hud_selection_index();
  *(undefined1 *)(unaff_A5 + unaff_D4w) = extraout_D1b;
  return;
}


