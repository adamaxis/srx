// Ghidra headless export
// Entry: 0x0000563E
// Name: clone_entity_spawn_variant_b
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void clone_entity_spawn_variant_b(void)

{
  int in_A0;
  int in_A1;
  
  *(undefined1 *)(in_A1 + 0xb2) = 6;
  *(undefined2 *)(in_A1 + 0x32) = *(undefined2 *)(in_A0 + 0x32);
  *(undefined2 *)(in_A1 + 0x2a) = *(undefined2 *)(in_A0 + 0x2a);
  *(undefined1 *)(in_A1 + 2) = 5;
  *(undefined1 *)(in_A1 + 3) = 0;
  *(undefined4 *)(in_A1 + 0xd8) = *(undefined4 *)(in_A0 + 0xd8);
  return;
}


