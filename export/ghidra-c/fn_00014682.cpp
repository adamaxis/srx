// Ghidra headless export
// Entry: 0x00014682
// Name: select_obstruction_route
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00014854 can_bypass_obstruction
//   0x000148AA find_first_occupied_obstruction_tile
//   0x000148D2 choose_dominant_axis_directions
//   0x000149C0 advance_obstruction_route_queue
//   0x00014AF4 find_rotating_route_strip_target


void select_obstruction_route(void)

{
  short sVar1;
  short extraout_D1w;
  undefined2 extraout_D1w_00;
  undefined2 unaff_D4w;
  short unaff_D5w;
  int in_A0;
  int extraout_A0;
  int extraout_A0_00;
  int extraout_A0_01;
  short *psVar2;
  undefined2 *puVar3;
  bool bVar4;
  
  *(byte *)(in_A0 + 0xb0) = *(byte *)(in_A0 + 0xb0) | 2;
  if ((*(byte *)(in_A0 + 0xb0) & 8) != 0) {
    return;
  }
  choose_dominant_axis_directions();
  if ((&DAT_00ff1700)[unaff_D5w] != '\0') {
    *(byte *)(extraout_A0 + 0xb0) = *(byte *)(extraout_A0 + 0xb0) & 0xfd;
    *(byte *)(extraout_A0 + 0xb0) = *(byte *)(extraout_A0 + 0xb0) & 0xf7;
    return;
  }
  *(undefined2 *)(extraout_A0 + 0xba) = 0;
  sVar1 = find_first_occupied_obstruction_tile();
  if (extraout_D1w != unaff_D5w) {
    psVar2 = (short *)((int)*(short *)(extraout_A0_00 + 0xba) + extraout_A0_00 + 0xbc);
    *psVar2 = unaff_D5w + sVar1;
    psVar2[1] = -1;
    *(undefined2 *)(extraout_A0_00 + 0xba) = 0;
    return;
  }
  bVar4 = false;
  can_bypass_obstruction();
  if (!bVar4) {
    puVar3 = (undefined2 *)((int)*(short *)(extraout_A0_01 + 0xba) + extraout_A0_01 + 0xbc);
    *puVar3 = extraout_D1w_00;
    puVar3[1] = unaff_D4w;
    puVar3[2] = 0xffff;
    *(undefined2 *)(extraout_A0_01 + 0xba) = 0;
    return;
  }
  *(byte *)(extraout_A0_01 + 0xb0) = *(byte *)(extraout_A0_01 + 0xb0) | 8;
  find_rotating_route_strip_target();
  advance_obstruction_route_queue();
  return;
}


