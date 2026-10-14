// Ghidra headless export
// Entry: 0x00014AF4
// Name: find_rotating_route_strip_target
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00014756 clear_entity_route_flags
//   0x00014800 probe_obstruction_strip
//   0x00014902 compute_obstruction_grid_index


void find_rotating_route_strip_target(void)

{
  short sVar1;
  short extraout_D1w;
  short unaff_D5w;
  int extraout_A0;
  
  compute_obstruction_grid_index();
  sVar1 = 4;
  do {
    probe_obstruction_strip();
    if (extraout_D1w != unaff_D5w) {
      *(undefined2 *)(extraout_A0 + 0xba) = 0;
      *(short *)(extraout_A0 + 0xbc) = extraout_D1w;
      *(undefined2 *)(extraout_A0 + 0xbe) = 0xffff;
      return;
    }
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  clear_entity_route_flags();
  return;
}


