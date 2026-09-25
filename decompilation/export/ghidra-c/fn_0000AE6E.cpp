// Ghidra headless export
// Entry: 0x0000AE6E
// Name: update_entity_target_heading
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0001033E FUN_0001033e
//   0x00014D8E FUN_00014d8e
//   0x00015D3C measure_relative_heading_and_distance


void update_entity_target_heading(void)

{
  undefined2 uVar1;
  uint extraout_D1;
  int in_A0;
  uint extraout_A0;
  bool bVar2;
  
  if (DAT_00ffe186 == '\x03') {
    DAT_00fff0dd = 0;
    return;
  }
  if (*(int *)(in_A0 + 0xa2) != 0) {
    uVar1 = *(undefined2 *)(*(int *)(in_A0 + 0xa2) + 4);
    measure_relative_heading_and_distance();
    *(undefined2 *)(extraout_A0 + 0x2e) = uVar1;
    bVar2 = extraout_D1 < extraout_A0;
    if ((extraout_D1 != extraout_A0) && (FUN_00014d8e(), bVar2)) {
      DAT_00fff0dd = 0;
      return;
    }
  }
  FUN_0001033e();
  return;
}


