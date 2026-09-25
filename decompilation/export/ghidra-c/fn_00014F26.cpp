// Ghidra headless export
// Entry: 0x00014F26
// Name: probe_entity_world_position
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000E7C8 evaluate_terrain_collision


void probe_entity_world_position(void)

{
  int *in_A0;
  
  evaluate_terrain_collision
            (*in_A0 + *(short *)(in_A0 + 0x13) * 0x100,
             in_A0[1] + *(short *)((int)in_A0 + 0x4e) * 0x100);
  return;
}


