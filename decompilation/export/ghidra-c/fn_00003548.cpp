// Ghidra headless export
// Entry: 0x00003548
// Name: draw_hud_panel_triplet
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00003560 refresh_hud_panel_variant


undefined8 draw_hud_panel_triplet(void)

{
  undefined4 in_D0;
  short sVar1;
  undefined4 in_D1;
  
  sVar1 = 2;
  do {
    refresh_hud_panel_variant();
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  return CONCAT44(in_D0,in_D1);
}


