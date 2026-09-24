// Ghidra headless export
// Entry: 0x0000AC84
// Name: dispatch_entity_collision_behavior
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000AE5E schedule_entity_collision_recheck
//   0x000138B8 FUN_000138b8
//   0x00013904 FUN_00013904


void dispatch_entity_collision_behavior(void)

{
  int in_A0;
  
  if ((*(byte *)(in_A0 + 0x10) & 8) == 0) {
    FUN_00013904();
  }
  else {
    FUN_000138b8();
  }
  if (DAT_00ffe18a != '\0') {
    schedule_entity_collision_recheck();
  }
  return;
}


