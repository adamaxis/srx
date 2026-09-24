// Ghidra headless export
// Entry: 0x0001E0DC
// Name: spawn_world_effect_entities
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x000044BC initialize_entity_record_from_template
//   0x00005A56 project_entity_screen_position
//   0x0001A2CC allocate_first_free_effect_slot
//   0x0001A342 find_first_available_world_effect_entity_slot


void spawn_world_effect_entities(void)

{
  uint uVar1;
  uint uVar2;
  undefined2 uVar3;
  undefined2 *extraout_A0;
  undefined4 extraout_A1;
  int extraout_A1_00;
  int extraout_A1_01;
  int iVar4;
  uint *puVar5;
  uint *puVar6;
  undefined4 unaff_A6;
  bool bVar7;
  
  puVar5 = *(uint **)(DAT_00ffde1e + 0x10);
  while ((bVar7 = false, *puVar5 != 0 &&
         (uVar3 = find_first_available_world_effect_entity_slot(), !bVar7))) {
    bVar7 = false;
    DAT_00ffdd40 = uVar3;
    allocate_first_free_effect_slot();
    if (bVar7) {
      return;
    }
    puVar6 = puVar5 + 1;
    uVar1 = *puVar5;
    puVar5 = puVar5 + 2;
    uVar2 = *puVar6;
    if ((uVar1 & 0x80000000) == 0) {
      DAT_00ffdd42 = DAT_00ffdd42 + 0xc0;
      initialize_entity_record_from_template();
      *(undefined4 *)(extraout_A1_01 + 0xa8) = unaff_A6;
      iVar4 = extraout_A1_01;
    }
    else {
      DAT_00ffdd42 = 0;
      initialize_entity_record_from_template();
      *(byte *)(extraout_A1_00 + 0x10) = *(byte *)(extraout_A1_00 + 0x10) | 0x20;
      iVar4 = extraout_A1_00;
      unaff_A6 = extraout_A1;
    }
    *(byte *)(iVar4 + 0x9e) = *(byte *)(iVar4 + 0x9e) | 2;
    project_entity_screen_position();
    if (extraout_A0[6] != 0) {
      extraout_A0[0x72] = 4;
      *extraout_A0 = *(undefined2 *)(uVar2 + 2);
      extraout_A0[2] = *(undefined2 *)(uVar2 + 4);
    }
  }
  return;
}


