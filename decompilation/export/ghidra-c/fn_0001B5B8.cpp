// Ghidra headless export
// Entry: 0x0001B5B8
// Name: attach_behavior_descriptor_a
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0001B7D0 FUN_0001b7d0
//   0x000530DE service_audio_if_enabled


void attach_behavior_descriptor_a(void)

{
  int extraout_A0;
  
  service_audio_if_enabled();
  FUN_0001b7d0();
  *(undefined **)(extraout_A0 + 8) = &DAT_0001c330;
  *(undefined **)(extraout_A0 + 4) = &DAT_0001c33a;
  return;
}


