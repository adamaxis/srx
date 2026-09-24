// Ghidra headless export
// Entry: 0x000043B6
// Name: process_pending_entity_template_lists
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00004466 spawn_entity_template_list
//   0x0000A800 prepare_pending_entity_template_list
//   0x0001DBAA schedule_world_activity_timeout
//   0x0005BA4C thunk_FUN_0005ba50


void process_pending_entity_template_lists(void)

{
  if (DAT_00ffe186 == '\x02') {
    thunk_FUN_0005ba50();
  }
  prepare_pending_entity_template_list();
  if (DAT_00ffe124 != 0) {
    spawn_entity_template_list();
    DAT_00ffe124 = 0;
  }
  if (DAT_00ffe120 != (undefined *)0x0) {
    if (DAT_00ffe120 == &DAT_0001e628) {
      DAT_00ffe12e = &UNK_00053ffc;
      schedule_world_activity_timeout();
      DAT_00ffe177 = 4;
    }
    else if (DAT_00ffe120 == &UNK_0001e5c4) {
      DAT_00ffe12e = &UNK_00054286;
      schedule_world_activity_timeout();
      DAT_00ffe177 = 4;
    }
    spawn_entity_template_list();
    DAT_00ffe120 = (undefined *)0x0;
    spawn_entity_template_list();
    return;
  }
  spawn_entity_template_list();
  return;
}


