// Ghidra headless export
// Entry: 0x000038FA
// Name: run_entity_animation_triplet
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00003674 advance_entity_delay_meter
//   0x000036FC advance_entity_asset_meter
//   0x000037EC advance_entity_behavior_meter


void run_entity_animation_triplet(void)

{
  short sVar1;
  
  sVar1 = 2;
  do {
    advance_entity_delay_meter();
    advance_entity_behavior_meter();
    advance_entity_asset_meter();
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  return;
}


