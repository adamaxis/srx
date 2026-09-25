// Ghidra headless export
// Entry: 0x0001B628
// Name: attach_behavior_descriptor_c
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0001B7D0 convert_world_routine_0x0001b7d0


void attach_behavior_descriptor_c(void)

{
  int extraout_A0;
  
  convert_world_routine_0x0001b7d0();
  *(undefined **)(extraout_A0 + 8) = &DAT_0001c2c8;
  *(undefined **)(extraout_A0 + 4) = &DAT_0001c2d2;
  return;
}


