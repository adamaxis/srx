// Ghidra headless export
// Entry: 0x0001E04A
// Name: begin_world_effect_transition
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0001DBAA schedule_world_activity_timeout
//   0x0001DF00 clear_entity_effect_pending_flags
//   0x0001E0DC spawn_world_effect_entities
//   0x000530DE FUN_000530de


void begin_world_effect_transition(void)

{
  if (DAT_00ffddc4 == 0x14) {
    schedule_world_activity_timeout();
  }
  else if ((DAT_00ffdf40 == 2) && (DAT_00ffe11e = 0x960, DAT_00ffe16f == '\0')) {
    DAT_00ffe16f = -1;
    FUN_000530de();
    clear_entity_effect_pending_flags();
    spawn_world_effect_entities();
  }
  return;
}


