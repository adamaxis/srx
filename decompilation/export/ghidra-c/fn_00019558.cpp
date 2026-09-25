// Ghidra headless export
// Entry: 0x00019558
// Name: reset_menu_visual_slots
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0001952C convert_world_routine_0x0001952c
//   0x0001954A convert_world_routine_0x0001954a


void reset_menu_visual_slots(void)

{
  short sVar1;
  
  sVar1 = 4;
  do {
    convert_world_routine_0x0001954a();
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  sVar1 = 0xb;
  do {
    convert_world_routine_0x0001952c();
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  return;
}


