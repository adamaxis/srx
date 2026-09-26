// Ghidra headless export
// Entry: 0x00013BBA
// Name: check_entity_animation_transition
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00013C20 select_entity_action_lane
//   0x00013C74 collect_entity_action_records


void check_entity_animation_transition(void)

{
  byte bVar1;
  short unaff_D3w;
  int in_A0;
  bool bVar2;
  
  bVar1 = *(byte *)(in_A0 + 0x57);
  if ((((bVar1 == 1) || (bVar1 == 2)) || (bVar1 == 8)) || (bVar1 == 9)) {
    collect_entity_action_records();
    bVar2 = false;
    if ((unaff_D3w != 0) && (select_entity_action_lane(), bVar2)) {
      return;
    }
  }
  else {
    bVar2 = bVar1 < 0xc;
    if (bVar1 != 0xc) {
      return;
    }
    select_entity_action_lane();
    if (!bVar2) {
      return;
    }
  }
  return;
}

