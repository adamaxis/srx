// Ghidra headless export
// Entry: 0x0001055E
// Name: refresh_equipment_combat_values
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001232 write_vdp_grid
//   0x0001047E refresh_ready_entity_animations
//   0x000104C8 find_matching_menu_metric
//   0x00055F02 calculate_item18_skill_value
//   0x00055F20 select_posture_adjusted_value


void refresh_equipment_combat_values(void)

{
  short in_D0w;
  char cVar1;
  int extraout_A0;
  
  if (in_D0w == 0) {
    calculate_item18_skill_value();
    select_posture_adjusted_value();
    find_matching_menu_metric();
    cVar1 = '\0';
  }
  else {
    refresh_ready_entity_animations();
    cVar1 = *(char *)(extraout_A0 + 0x7c);
  }
  while (write_vdp_grid(), cVar1 != '\0') {
    cVar1 = '\0';
  }
  return;
}


