// Ghidra headless export
// Entry: 0x00055640
// Name: convert_intro_routine_0x00055640
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000D5A guard_entity_phase_window
//   0x00000DBA count_thresholded_random_rolls
//   0x00012C74 convert_menu_routine_0x00012c74
//   0x000556C6 convert_intro_routine_0x000556c6
//   0x00055740 convert_intro_routine_0x00055740
//   0x00055786 convert_intro_routine_0x00055786
//   0x00055ED6 convert_intro_routine_0x00055ed6
//   0x00055F20 convert_intro_routine_0x00055f20


undefined2 convert_intro_routine_0x00055640(void)

{
  undefined2 in_D0w;
  int extraout_A0;
  int extraout_A1;
  
  convert_intro_routine_0x00055ed6();
  convert_intro_routine_0x00055f20();
  convert_menu_routine_0x00012c74();
  convert_intro_routine_0x00055740();
  convert_intro_routine_0x00055786();
  convert_intro_routine_0x000556c6();
  guard_entity_phase_window();
  count_thresholded_random_rolls();
  *(undefined1 *)(extraout_A0 + 0x9f) = DAT_00fff0dd;
  *(undefined1 *)(extraout_A1 + 0x9f) = DAT_00fff0dd;
  return in_D0w;
}


