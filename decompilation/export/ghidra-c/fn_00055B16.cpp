// Ghidra headless export
// Entry: 0x00055B16
// Name: convert_intro_routine_0x00055b16
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000D5A guard_entity_phase_window
//   0x00000DBA count_thresholded_random_rolls
//   0x0001047E refresh_ready_entity_animations
//   0x00055740 convert_intro_routine_0x00055740
//   0x00055786 convert_intro_routine_0x00055786
//   0x000559BC convert_intro_routine_0x000559bc


/* WARNING: Removing unreachable block (ram,0x00055b36) */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void convert_intro_routine_0x00055b16(void)

{
  int extraout_A0;
  int extraout_A1;
  
  convert_intro_routine_0x00055740();
  convert_intro_routine_0x00055786();
  guard_entity_phase_window();
  count_thresholded_random_rolls();
  *(undefined1 *)(extraout_A0 + 0x9f) = DAT_00fff0dd;
  *(undefined1 *)(extraout_A0 + 0x9d) = 6;
  if ((short)(*(short *)(extraout_A1 + 0x3e) + -3) < 0) {
    refresh_ready_entity_animations();
  }
  _DAT_00fff112 = 1;
  convert_intro_routine_0x000559bc();
  return;
}


