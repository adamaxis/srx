// Ghidra headless export
// Entry: 0x00013AE6
// Name: advance_entity_action_threshold
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00013466 set_entity_action_state
//   0x00013AF2 find_first_entity_action_below_threshold


void advance_entity_action_threshold(void)

{
  find_first_entity_action_below_threshold();
  set_entity_action_state();
  return;
}


