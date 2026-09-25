// Ghidra headless export
// Entry: 0x00005468
// Name: clone_entity_spawn_variant_a
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void clone_entity_spawn_variant_a(void)

{
  int in_A0;
  int in_A1;
  
  DAT_00ffe004 = 4;
  *(undefined1 *)(in_A1 + 0xb2) = 0xd;
  *(undefined2 *)(in_A1 + 0x32) = *(undefined2 *)(in_A0 + 0x32);
  *(undefined2 *)(in_A1 + 0x2a) = *(undefined2 *)(in_A0 + 0x2a);
  *(undefined1 *)(in_A1 + 2) = 5;
  *(undefined1 *)(in_A1 + 3) = 0;
  *(undefined4 *)(in_A1 + 0xd8) = *(undefined4 *)(in_A0 + 0xd8);
  return;
}


