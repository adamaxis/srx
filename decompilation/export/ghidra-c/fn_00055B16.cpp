// Ghidra headless export
// Entry: 0x00055B16
// Name: resolve_entity_action_with_default_bonus
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000D5A guard_entity_phase_window
//   0x00000DBA count_thresholded_random_rolls
//   0x0001047E refresh_ready_entity_animations
//   0x00055740 apply_target_condition_modifier
//   0x00055786 apply_dynamic_status_modifier
//   0x000559BC resolve_dynamic_entity_action


/* WARNING: Removing unreachable block (ram,0x00055b36) */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void resolve_entity_action_with_default_bonus(void)

{
  int extraout_A0;
  int extraout_A1;
  
  apply_target_condition_modifier();
  apply_dynamic_status_modifier();
  guard_entity_phase_window();
  count_thresholded_random_rolls();
  *(undefined1 *)(extraout_A0 + 0x9f) = DAT_00fff0dd;
  *(undefined1 *)(extraout_A0 + 0x9d) = 6;
  if ((short)(*(short *)(extraout_A1 + 0x3e) + -3) < 0) {
    refresh_ready_entity_animations();
  }
  _DAT_00fff112 = 1;
  resolve_dynamic_entity_action();
  return;
}


