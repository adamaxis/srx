// Ghidra headless export
// Entry: 0x0005D3E6
// Name: spawn_world_scene_secondary_effect
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000AFF2 prepare_entity_auxiliary_behavior
//   0x00015930 apply_entity_secondary_effect


undefined8 spawn_world_scene_secondary_effect(void)

{
  undefined4 in_D0;
  undefined4 in_D1;
  int in_A1;
  
  *(byte *)(in_A1 + 0x11) = *(byte *)(in_A1 + 0x11) | 1;
  apply_entity_secondary_effect();
  prepare_entity_auxiliary_behavior();
  return CONCAT44(in_D0,in_D1);
}


