// Ghidra headless export
// Entry: 0x00014756
// Name: clear_entity_route_flags
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void clear_entity_route_flags(void)

{
  int in_A0;
  
  *(byte *)(in_A0 + 0xb0) = *(byte *)(in_A0 + 0xb0) & 0xfd;
  *(byte *)(in_A0 + 0xb0) = *(byte *)(in_A0 + 0xb0) & 0xf7;
  return;
}


