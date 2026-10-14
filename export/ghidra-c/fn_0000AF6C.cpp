// Ghidra headless export
// Entry: 0x0000AF6C
// Name: reset_entity_animation_state
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void reset_entity_animation_state(void)

{
  int in_A0;
  
  *(undefined2 *)(in_A0 + 0x1a) = 0;
  *(byte *)(in_A0 + 0x11) = *(byte *)(in_A0 + 0x11) | 1;
  *(undefined2 *)(in_A0 + 0x1e) = 0;
  return;
}


