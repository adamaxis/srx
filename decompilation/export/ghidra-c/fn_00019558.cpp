// Ghidra headless export
// Entry: 0x00019558
// Name: reset_menu_visual_slots
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0001952C FUN_0001952c
//   0x0001954A FUN_0001954a


void reset_menu_visual_slots(void)

{
  short sVar1;
  
  sVar1 = 4;
  do {
    FUN_0001954a();
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  sVar1 = 0xb;
  do {
    FUN_0001952c();
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  return;
}


