// Ghidra headless export
// Entry: 0x00014226
// Name: FUN_00014226
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x000036FC FUN_000036fc
//   0x0000ABFC update_entity_animation_delay
//   0x000530DE service_audio_if_enabled


void FUN_00014226(void)

{
  int extraout_A0;
  
  service_audio_if_enabled();
  update_entity_animation_delay();
  *(char *)(extraout_A0 + 0x4f) = *(char *)(extraout_A0 + 0x4f) + -1;
  FUN_000036fc();
  return;
}


