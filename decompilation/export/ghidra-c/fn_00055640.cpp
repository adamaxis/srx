// Ghidra headless export
// Entry: 0x00055640
// Name: resolve_entity_action_pair
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000D5A guard_entity_phase_window
//   0x00000DBA count_thresholded_random_rolls
//   0x00012C74 merge_equipment_action_skill_value
//   0x000556C6 check_entity_action_eligibility
//   0x00055740 apply_target_condition_modifier
//   0x00055786 apply_dynamic_status_modifier
//   0x00055ED6 calculate_equipment_command_average
//   0x00055F20 select_posture_adjusted_value


undefined2 resolve_entity_action_pair(void)

{
  undefined2 in_D0w;
  int extraout_A0;
  int extraout_A1;
  
  calculate_equipment_command_average();
  select_posture_adjusted_value();
  merge_equipment_action_skill_value();
  apply_target_condition_modifier();
  apply_dynamic_status_modifier();
  check_entity_action_eligibility();
  guard_entity_phase_window();
  count_thresholded_random_rolls();
  *(undefined1 *)(extraout_A0 + 0x9f) = DAT_00fff0dd;
  *(undefined1 *)(extraout_A1 + 0x9f) = DAT_00fff0dd;
  return in_D0w;
}


