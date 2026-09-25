// Ghidra headless export
// Entry: 0x0001DD94
// Name: advance_world_frame_logic
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0001DBF0 reset_entity_service_records
//   0x0001DC4C convert_world_routine_0x0001dc4c
//   0x0001DEA0 clear_inactive_entity_effects
//   0x0001DF1A resolve_nearby_effects
//   0x0001DFC4 advance_world_effect_trigger
//   0x0001E166 trigger_world_effect_sequence
//   0x0001E2EC begin_world_activity
//   0x00053126 prepare_mode_entry_service
//   0x00016ECA process_terrain_work_batch

/* One world-logic frame. The native conversion keeps this ordering as a
 * typed coordinator; it does not reproduce the source's RAM layout. */

void advance_world_frame_logic(void)
{
  if (DAT_00ffe186 == 2 && (short)DAT_00fff13e._2_2_ < 0 &&
      DAT_00ff044c <= 9) {
    DAT_00fff13e._2_2_ = 0;
    DAT_00ffe186 = 3;
  }

  if (DAT_00ffe166 != 0 && --DAT_00ffe166 == 0) {
    prepare_mode_entry_service();
    DAT_00ffdd2c = convert_graphics_routine_0x0000c64c;
    begin_world_activity();
  }
  if (DAT_00ffe11c != 0) {
    --DAT_00ffe11c;
  }

  if (DAT_00ffdf84 != 0) {
    if (DAT_00ffe186 == 0 && DAT_00ffe16f == 0 &&
        DAT_00ffddc4 != 0x14) {
      if (--DAT_00ffdf86 < 0) {
        reset_entity_service_records();
      }
    }
    else if (current_entity_target_is_missing_or_locked()) {
      convert_world_routine_0x0001dc4c();
    }
  }

  clear_inactive_entity_effects();
  if (DAT_00ffdf40 == 2) {
    process_terrain_work_batch();
    advance_world_effect_trigger();
  }
  trigger_world_effect_sequence();
  resolve_nearby_effects();
}
