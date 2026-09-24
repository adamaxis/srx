// Ghidra headless export
// Entry: 0x0000BADE
// Name: advance_entity_movement_step
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000B442 reset_entity_movement_state
//   0x0000B732 apply_movement_result_policy
//   0x0000B77E probe_entity_directional_collision
//   0x0000B7B8 FUN_0000b7b8
//   0x0000BB86 evaluate_entity_movement_route
//   0x0000BBE4 commit_entity_position


void advance_entity_movement_step(void)

{
  short sVar1;
  undefined2 unaff_D5w;
  undefined2 unaff_D6w;
  int in_A0;
  int extraout_A0;
  int iVar2;
  int extraout_A0_00;
  undefined2 *extraout_A0_01;
  undefined2 *extraout_A0_02;
  undefined2 *extraout_A0_03;
  undefined2 *puVar3;
  int in_A1;
  bool bVar4;
  
  *(undefined2 *)(in_A0 + 0xca) = *(undefined2 *)(in_A1 + 4);
  sVar1 = probe_entity_directional_collision();
  iVar2 = extraout_A0;
  if ((sVar1 == 0) ||
     ((sVar1 < 0 &&
      (FUN_0000b7b8(), iVar2 = extraout_A0_00, (*(byte *)(extraout_A0_00 + 0xb0) & 1) == 0)))) {
    *(undefined2 *)(iVar2 + 0xb8) = 0x1000;
    commit_entity_position();
    return;
  }
  bVar4 = false;
  if ((*(ushort *)(iVar2 + 0x2e) & 0x3fff) == 0) {
    evaluate_entity_movement_route();
    puVar3 = extraout_A0_03;
    if (!bVar4) {
      *(byte *)(extraout_A0_03 + 0x58) = *(byte *)(extraout_A0_03 + 0x58) & 0xfe;
      *extraout_A0_03 = unaff_D5w;
      extraout_A0_03[2] = unaff_D6w;
      return;
    }
LAB_0000bb66:
    puVar3[0x5c] = 0xffff;
    reset_entity_movement_state();
    return;
  }
  DAT_00ffdd3c = *(ushort *)(iVar2 + 0x2e) >> 0xd & 6;
  *(ushort *)(iVar2 + 0xb8) = DAT_00ffdd3c;
  sVar1 = apply_movement_result_policy();
  puVar3 = extraout_A0_01;
  if ((sVar1 != 0) && (-1 < sVar1)) {
    extraout_A0_01[0x5c] = DAT_00ffdd3c;
    sVar1 = apply_movement_result_policy();
    puVar3 = extraout_A0_02;
    if ((sVar1 != 0) && (-1 < sVar1)) goto LAB_0000bb66;
  }
  puVar3[0x1b] = DAT_00ffdf66 << 0xc | DAT_00ffdf66 >> 4;
  *(byte *)((int)puVar3 + 0x11) = *(byte *)((int)puVar3 + 0x11) | 4;
  commit_entity_position();
  return;
}


