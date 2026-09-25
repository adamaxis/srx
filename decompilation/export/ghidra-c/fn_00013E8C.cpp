// Ghidra headless export
// Entry: 0x00013E8C
// Name: service_entity_boundary_transition
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00013E9E reposition_entity_at_boundary
//   0x000145A2 convert_entity_routine_0x000145a2


void service_entity_boundary_transition(void)

{
  int in_A0;
  
  if (*(short *)(in_A0 + 0xc) != 0) {
    reposition_entity_at_boundary();
    convert_entity_routine_0x000145a2();
    return;
  }
  if (0x3c < *(short *)(in_A0 + 0x16)) {
    return;
  }
  *(undefined4 *)(in_A0 + 0x3a) = 0xb45c;
  return;
}


