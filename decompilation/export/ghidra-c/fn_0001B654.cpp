// Ghidra headless export
// Entry: 0x0001B654
// Name: attach_behavior_descriptor_d
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0001B7D0 convert_world_routine_0x0001b7d0


void attach_behavior_descriptor_d(void)

{
  int extraout_A0;
  
  convert_world_routine_0x0001b7d0();
  *(undefined **)(extraout_A0 + 8) = &DAT_0001c318;
  *(undefined **)(extraout_A0 + 4) = &DAT_0001c31e;
  return;
}


