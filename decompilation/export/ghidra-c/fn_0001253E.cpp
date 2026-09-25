// Ghidra headless export
// Entry: 0x0001253E
// Name: convert_menu_routine_0x0001253e
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000DBA count_thresholded_random_rolls
//   0x00003674 advance_entity_delay_meter
//   0x000037EC advance_entity_behavior_meter


void convert_menu_routine_0x0001253e(void)

{
  int extraout_A0;
  int in_A1;
  
  if ((*(char *)(in_A1 + 0x4c) == '\n') && (*(char *)(in_A1 + 0x9c) == '\n')) {
    return;
  }
  count_thresholded_random_rolls();
  advance_entity_delay_meter();
  advance_entity_behavior_meter();
  *(byte *)(extraout_A0 + 0x10) = *(byte *)(extraout_A0 + 0x10) & 0xbf;
  *(byte *)(extraout_A0 + 0x10) = *(byte *)(extraout_A0 + 0x10) & 0xef;
  *(byte *)(extraout_A0 + 0x10) = *(byte *)(extraout_A0 + 0x10) | 1;
  *(undefined4 *)(extraout_A0 + 0x3a) = 0xb45c;
  return;
}


