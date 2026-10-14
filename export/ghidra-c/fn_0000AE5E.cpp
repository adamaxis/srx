// Ghidra headless export
// Entry: 0x0000AE5E
// Name: schedule_entity_collision_recheck
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void schedule_entity_collision_recheck(void)

{
  int in_A0;
  
  *(byte *)(in_A0 + 0x11) = *(byte *)(in_A0 + 0x11) | 1;
  *(undefined4 *)(in_A0 + 0x3a) = 0xb45c;
  return;
}


