// Ghidra headless export
// Entry: 0x00003548
// Name: draw_hud_panel_triplet
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00003560 FUN_00003560


undefined8 draw_hud_panel_triplet(void)

{
  undefined4 in_D0;
  short sVar1;
  undefined4 in_D1;
  
  sVar1 = 2;
  do {
    FUN_00003560();
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  return CONCAT44(in_D0,in_D1);
}


