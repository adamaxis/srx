// Ghidra headless export
// Entry: 0x0001B612
// Name: attach_behavior_descriptor_b
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0001B7D0 initialize_behavior_record


void attach_behavior_descriptor_b(void)

{
  int extraout_A0;
  
  initialize_behavior_record();
  *(undefined **)(extraout_A0 + 8) = &DAT_0001c294;
  *(undefined **)(extraout_A0 + 4) = &DAT_0001c29e;
  return;
}

