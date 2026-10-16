// Ghidra headless export
// Entry: 0x00015C58
// Name: is_entity_target_within_range
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00014D58 measure_entity_target_heading_and_distance


void is_entity_target_within_range(void)

{
  ushort unaff_D3w;
  int in_A1;
  
  if (((*(byte *)(in_A1 + 0x10) & 1) != 0) && ((*(byte *)(in_A1 + 0x10) & 0x40) == 0)) {
    measure_entity_target_heading_and_distance();
    if (unaff_D3w < 0x200) {
      return;
    }
  }
  return;
}


