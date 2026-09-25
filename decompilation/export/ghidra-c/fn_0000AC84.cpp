// Ghidra headless export
// Entry: 0x0000AC84
// Name: dispatch_entity_collision_behavior
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000AE5E schedule_entity_collision_recheck
//   0x000138B8 convert_menu_routine_0x000138b8
//   0x00013904 convert_menu_routine_0x00013904


void dispatch_entity_collision_behavior(void)

{
  int in_A0;
  
  if ((*(byte *)(in_A0 + 0x10) & 8) == 0) {
    convert_menu_routine_0x00013904();
  }
  else {
    convert_menu_routine_0x000138b8();
  }
  if (DAT_00ffe18a != '\0') {
    schedule_entity_collision_recheck();
  }
  return;
}


