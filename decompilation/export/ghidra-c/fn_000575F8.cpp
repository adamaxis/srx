// Ghidra headless export
// Entry: 0x000575F8
// Name: convert_intro_routine_0x000575f8
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0001952C convert_world_routine_0x0001952c
//   0x00019558 reset_menu_visual_slots
//   0x00057610 convert_intro_routine_0x00057610


void convert_intro_routine_0x000575f8(void)

{
  reset_menu_visual_slots();
  convert_intro_routine_0x00057610();
  convert_world_routine_0x0001952c();
  return;
}


