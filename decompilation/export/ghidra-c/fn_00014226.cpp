// Ghidra headless export
// Entry: 0x00014226
// Name: service_entity_animation_action
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x000036FC advance_entity_asset_meter
//   0x0000ABFC update_entity_animation_delay
//   0x000530DE service_audio_if_enabled


void service_entity_animation_action(void)

{
  int extraout_A0;
  
  service_audio_if_enabled();
  update_entity_animation_delay();
  *(char *)(extraout_A0 + 0x4f) = *(char *)(extraout_A0 + 0x4f) + -1;
  advance_entity_asset_meter();
  return;
}

