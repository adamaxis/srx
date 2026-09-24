// Ghidra headless export
// Entry: 0x0001E25A
// Name: spawn_world_effect_from_coordinate_sequence
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x000044BC initialize_entity_record_from_template
//   0x0001A2CC allocate_first_free_effect_slot
//   0x0001A342 find_first_available_world_effect_entity_slot
//   0x0001DBAA schedule_world_activity_timeout


void spawn_world_effect_from_coordinate_sequence(void)

{
  short sVar1;
  undefined2 uVar2;
  short sVar3;
  short *extraout_A1;
  short *unaff_A4;
  short *psVar4;
  bool bVar5;
  
  bVar5 = false;
  DAT_00ffdd4c = unaff_A4;
  uVar2 = find_first_available_world_effect_entity_slot();
  if (!bVar5) {
    bVar5 = false;
    DAT_00ffdd40 = uVar2;
    allocate_first_free_effect_slot();
    if (!bVar5) {
      initialize_entity_record_from_template();
      *(undefined1 *)((int)extraout_A1 + 0x4d) = 4;
      *(byte *)(extraout_A1 + 8) = *(byte *)(extraout_A1 + 8) | 0x20;
      psVar4 = DAT_00ffdd4c + 1;
      sVar3 = *DAT_00ffdd4c;
      if (*DAT_00ffdd4c == 0) {
        psVar4 = (short *)&DAT_00ffdd52;
        sVar3 = DAT_00ffdd50;
      }
      DAT_00ffdd4c = psVar4 + 1;
      sVar1 = *psVar4;
      *extraout_A1 = sVar3;
      extraout_A1[2] = sVar1;
      DAT_00ffe177 = DAT_00ffe177 + '\x01';
      *(byte *)(extraout_A1 + 0x4f) = *(byte *)(extraout_A1 + 0x4f) | 0x40;
      extraout_A1[0x4b] = *extraout_A1;
      extraout_A1[0x4c] = extraout_A1[2];
      schedule_world_activity_timeout();
      return;
    }
  }
  return;
}


