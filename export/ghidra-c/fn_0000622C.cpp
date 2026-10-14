// Ghidra headless export
// Entry: 0x0000622C
// Name: expand_entity_sprite_descriptors
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000624C expand_entity_sprite_descriptor_list


void expand_entity_sprite_descriptors(void)

{
  int in_A0;
  
  if (**(short **)(in_A0 + 0x12) != -0x7ffe) {
    expand_entity_sprite_descriptor_list();
    return;
  }
  expand_entity_sprite_descriptor_list();
  return;
}


