// Ghidra headless export
// Entry: 0x0000B1DC
// Name: configure_entity_target_transition
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000ABFC update_entity_animation_delay
//   0x0000AF6C reset_entity_animation_state
//   0x0000AFF2 prepare_entity_auxiliary_behavior
//   0x00015A8C find_first_available_entity_slot
//   0x00015C84 update_entity_heading_from_linked_target
//   0x00015D3C measure_relative_heading_and_distance
//   0x0001DBAA schedule_world_activity_timeout
//   0x000530DE FUN_000530de
//   0x00055C76 FUN_00055c76


void configure_entity_target_transition(void)

{
  ushort uVar1;
  byte bVar3;
  undefined2 uVar2;
  int extraout_A0;
  int extraout_A0_00;
  int iVar4;
  undefined4 extraout_A0_01;
  undefined2 *extraout_A0_02;
  int extraout_A1;
  int extraout_A1_00;
  undefined2 *extraout_A1_01;
  undefined1 uVar5;
  undefined8 uVar6;
  undefined2 in_stack_fffffffa;
  undefined4 uVar7;
  
  update_entity_animation_delay();
  FUN_000530de();
  iVar4 = extraout_A0;
  if (*(int *)(extraout_A0 + 0xa2) != 0) {
    uVar2 = *(undefined2 *)(*(int *)(extraout_A0 + 0xa2) + 4);
    measure_relative_heading_and_distance();
    *(undefined2 *)(extraout_A0_00 + 0x34) = uVar2;
    iVar4 = extraout_A0_00;
  }
  *(undefined2 *)(iVar4 + 0x2e) = *(undefined2 *)(iVar4 + 0x34);
  *(undefined1 **)(iVar4 + 0x3a) = &LAB_0000b316;
  uVar1 = *(ushort *)(iVar4 + 0x2e) << 3 | *(ushort *)(iVar4 + 0x2e) >> 0xd;
  if ((short)uVar1 < 0) {
    uVar1 = uVar1 + 1;
  }
  *(undefined2 *)(iVar4 + 0x16) = *(undefined2 *)(&DAT_0000c0a2 + (short)((uVar1 & 7) << 1));
  uVar5 = 0;
  reset_entity_animation_state();
  update_entity_heading_from_linked_target();
  if ((bool)uVar5) {
    schedule_world_activity_timeout();
    uVar2 = 0xb278;
    uVar6 = FUN_000530de();
    uVar7 = CONCAT22((short)CONCAT31((int3)((ulonglong)uVar6 >> 0x28),
                                     *(undefined1 *)(extraout_A1 + 0x9c)),in_stack_fffffffa);
    FUN_00055c76(uVar7,CONCAT22((short)CONCAT31((int3)((ulonglong)uVar6 >> 8),
                                                *(undefined1 *)(extraout_A1 + 0x4c)),uVar2));
    uVar5 = (byte)uVar7 < *(byte *)(extraout_A1_00 + 0x4c);
    if (((byte)uVar7 != *(byte *)(extraout_A1_00 + 0x4c)) ||
       (bVar3 = (byte)((uint)uVar7 >> 0x10), uVar5 = bVar3 < *(byte *)(extraout_A1_00 + 0x9c),
       bVar3 != *(byte *)(extraout_A1_00 + 0x9c))) {
      prepare_entity_auxiliary_behavior(extraout_A0_01,extraout_A1_00);
      uVar2 = find_first_available_entity_slot();
      if (!(bool)uVar5) {
        extraout_A1_01[0x1f] = uVar2;
        *(byte *)((int)extraout_A1_01 + 0x11) = *(byte *)((int)extraout_A1_01 + 0x11) | 1;
        extraout_A1_01[0xb] = 4;
        extraout_A1_01[7] = 1;
        *(undefined1 **)(extraout_A1_01 + 0x1d) = &LAB_0000ac4e;
        *(undefined1 *)(extraout_A1_01 + 8) = 1;
        *extraout_A1_01 = *extraout_A0_02;
        extraout_A1_01[2] = extraout_A0_02[2] + 0x10;
        extraout_A1_01[0x16] = 0x2000;
        *(byte *)((int)extraout_A1_01 + 0x11) = *(byte *)((int)extraout_A1_01 + 0x11) | 1;
      }
    }
  }
  return;
}


