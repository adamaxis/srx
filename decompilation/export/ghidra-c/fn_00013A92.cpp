// Ghidra headless export
// Entry: 0x00013A92
// Name: run_entity_animation_branch
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00013AE6 advance_entity_action_threshold
//   0x00013B12 run_entity_animation_transition
//   0x00013BBA check_entity_animation_transition
//   0x00013CD4 evaluate_entity_animation_rate_from_phase


void run_entity_animation_branch(void)

{
  int in_A0;
  int extraout_A0;
  int extraout_A0_00;
  bool bVar1;
  
  if (*(char *)(in_A0 + 0x9b) < '\x02') {
    bVar1 = false;
    if ((*(char *)(in_A0 + 0x56) < '\0') &&
       (check_entity_animation_transition(), in_A0 = extraout_A0, bVar1)) {
      evaluate_entity_animation_rate_from_phase();
      if (!bVar1) {
        return;
      }
      if (((-1 < *(char *)(extraout_A0_00 + 0x56)) && (*(char *)(extraout_A0_00 + 0x57) != '\0')) &&
         (*(char *)(extraout_A0_00 + 0x57) < '\r')) {
        return;
      }
      advance_entity_action_threshold();
      return;
    }
    bVar1 = *(byte *)(in_A0 + 0x9c) < 5;
    if ('\x04' < (char)*(byte *)(in_A0 + 0x9c)) {
      run_entity_animation_transition();
      if (bVar1) {
        find_first_entity_action_below_threshold();
        set_entity_action_state();
        return;
      }
      return;
    }
  }
  return;
}

