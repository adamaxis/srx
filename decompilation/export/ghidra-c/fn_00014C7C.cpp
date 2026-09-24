// Ghidra headless export
// Entry: 0x00014C7C
// Name: resolve_entity_target_motion
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00014C56 project_heading_position
//   0x00014D58 measure_entity_target_heading_and_distance
//   0x00015C58 is_entity_target_within_range
//   0x00015C84 update_entity_heading_from_linked_target
//   0x0001A2CC allocate_first_free_effect_slot


void resolve_entity_target_motion(void)

{
  undefined2 uVar1;
  undefined2 extraout_D1w;
  undefined2 unaff_D2w;
  ushort unaff_D3w;
  int in_A0;
  int extraout_A0;
  int iVar2;
  int extraout_A0_00;
  int extraout_A0_01;
  int extraout_A0_02;
  int extraout_A0_03;
  int extraout_A0_04;
  bool bVar3;
  byte bVar4;
  
  if (*(short *)(in_A0 + 0xe) != 0xc) {
    if ((*(short *)(in_A0 + 0xe) == 0xd) ||
       (bVar3 = *(ushort *)(in_A0 + 0xe) < 0xe, *(ushort *)(in_A0 + 0xe) == 0xe)) {
      measure_entity_target_heading_and_distance();
      DAT_00ffdd48 = *(undefined2 *)(extraout_A0_00 + 0x2e);
      project_heading_position();
      *(undefined2 *)(extraout_A0_01 + 0x2e) = unaff_D2w;
      bVar4 = 0;
      update_entity_heading_from_linked_target();
      *(undefined2 *)(extraout_A0_02 + 0x2e) = DAT_00ffdd48;
      if ((bVar4 & 1) != 0) {
        if ((*(short *)(extraout_A0_02 + 0xe) == 0xe) || (*(short *)(extraout_A0_02 + 0x1e) != 2)) {
          *(undefined2 *)(extraout_A0_02 + 0x2e) = unaff_D2w;
          *(undefined2 *)(extraout_A0_02 + 0x34) = unaff_D2w;
          iVar2 = extraout_A0_02;
          goto LAB_00014ca8;
        }
        *(undefined2 *)(extraout_A0_02 + 0x1a) = 0x1000;
      }
    }
    else {
      is_entity_target_within_range();
      iVar2 = extraout_A0;
      if (bVar3) goto LAB_00014ca8;
    }
    return;
  }
  measure_entity_target_heading_and_distance();
  iVar2 = extraout_A0_03;
  if (unaff_D3w < 0x300) {
    bVar3 = false;
    if (*(short *)(extraout_A0_03 + 0xdc) < 0) {
      uVar1 = allocate_first_free_effect_slot();
      if (bVar3) {
        return;
      }
      *(undefined2 *)(extraout_A0_04 + 0x32) = uVar1;
      *(undefined2 *)(extraout_A0_04 + 0x2a) = extraout_D1w;
      *(byte *)(extraout_A0_04 + 0x11) = *(byte *)(extraout_A0_04 + 0x11) | 0x40;
      *(undefined2 *)(extraout_A0_04 + 0x2e) = unaff_D2w;
      *(undefined2 *)(extraout_A0_04 + 0x34) = unaff_D2w;
      iVar2 = extraout_A0_04;
    }
  }
  else {
    if (0x5ff < unaff_D3w) {
      return;
    }
    *(undefined2 *)(extraout_A0_03 + 0x2e) = unaff_D2w;
    *(undefined2 *)(extraout_A0_03 + 0x34) = unaff_D2w;
  }
LAB_00014ca8:
  *(undefined2 *)(iVar2 + 0x34) = unaff_D2w;
  return;
}


