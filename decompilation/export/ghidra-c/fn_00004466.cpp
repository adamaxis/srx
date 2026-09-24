// Ghidra headless export
// Entry: 0x00004466
// Name: FUN_00004466
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x000044BC initialize_entity_record_from_template
//   0x0001A2CC allocate_first_free_effect_slot
//   0x0001A342 find_first_available_world_effect_entity_slot


void FUN_00004466(void)

{
  uint uVar1;
  undefined2 uVar2;
  undefined4 extraout_A1;
  int extraout_A1_00;
  int extraout_A1_01;
  uint *unaff_A5;
  undefined4 unaff_A6;
  bool bVar3;
  
  while ((bVar3 = false, *unaff_A5 != 0 &&
         (uVar2 = find_first_available_world_effect_entity_slot(), !bVar3))) {
    bVar3 = false;
    DAT_00ffdd40 = uVar2;
    allocate_first_free_effect_slot();
    if (bVar3) {
      return;
    }
    uVar1 = *unaff_A5;
    unaff_A5 = unaff_A5 + 2;
    if ((uVar1 & 0x80000000) == 0) {
      DAT_00ffdd42 = DAT_00ffdd42 + 0xc0;
      initialize_entity_record_from_template();
      *(undefined4 *)(extraout_A1_01 + 0xa8) = unaff_A6;
    }
    else {
      DAT_00ffdd42 = 0;
      initialize_entity_record_from_template();
      *(byte *)(extraout_A1_00 + 0x10) = *(byte *)(extraout_A1_00 + 0x10) | 0x20;
      unaff_A6 = extraout_A1;
    }
  }
  return;
}


