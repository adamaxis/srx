// Ghidra headless export
// Entry: 0x0001E4AC
// Name: spawn_world_effect_at_player_position
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x000044BC FUN_000044bc
//   0x0001A2CC allocate_first_free_effect_slot
//   0x0001A342 find_first_available_world_effect_entity_slot


void spawn_world_effect_at_player_position(void)

{
  short *psVar1;
  undefined2 uVar2;
  short *extraout_A1;
  undefined4 unaff_A6;
  bool bVar3;
  
  bVar3 = false;
  uVar2 = find_first_available_world_effect_entity_slot();
  if (!bVar3) {
    bVar3 = false;
    DAT_00ffdd40 = uVar2;
    allocate_first_free_effect_slot();
    if (!bVar3) {
      FUN_000044bc();
      *(byte *)(extraout_A1 + 0x58) = *(byte *)(extraout_A1 + 0x58) | 0x80;
      psVar1 = DAT_00fffc04;
      DAT_00fffc3c = unaff_A6;
      *extraout_A1 = *DAT_00fffc04 + 0x80;
      extraout_A1[2] = psVar1[2];
      extraout_A1[0x4b] = *extraout_A1;
      extraout_A1[0x4c] = extraout_A1[2];
    }
  }
  return;
}


