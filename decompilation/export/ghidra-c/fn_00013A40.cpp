// Ghidra headless export
// Entry: 0x00013A40
// Name: copy_entity_anchor_position
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void copy_entity_anchor_position(void)

{
  int in_A0;
  undefined2 *in_A1;
  
  *(undefined2 *)(in_A0 + 0x96) = *in_A1;
  *(undefined2 *)(in_A0 + 0x98) = in_A1[2];
  return;
}


