// Ghidra headless export
// Entry: 0x00002900
// Name: FUN_00002900
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001610 decode_source_byte_stream
//   0x0000313A copy_terrain_layers
//   0x00004388 finalize_pending_entity_template_update
//   0x00004B6C reset_effect_record
//   0x00005B7E mark_entity_slots_for_render
//   0x0000EE10 rebuild_terrain_collision_map
//   0x00013874 dispatch_entity_action_candidates
//   0x000164A8 populate_entity_table
//   0x000164DC clear_scene_record_transition_words
//   0x00016DEE clear_scene_record_pending_flags
//   0x00016E5C process_terrain_work_entries
//   0x0001A310 clear_effect_slots
//   0x0001A322 clear_entity_effect_pending_records
//   0x0001E04A begin_world_effect_transition
//   0x000530EA service_audio_command_frame
//   0x0005BA50 load_scene_resources
//   0x0005D260 FUN_0005d260


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00002900(void)

{
  if (DAT_00ffe172 != '\0') {
    DAT_00ffe172 = '\0';
    service_audio_command_frame();
  }
  if (_DAT_00ffdf8a != 0) {
    if (DAT_00ffdca0 != 0) {
      reset_effect_record();
    }
    DAT_00ffe16f = 0;
    if (DAT_00ffe191 != '\0') {
      DAT_00ffe191 = '\0';
      begin_world_effect_transition();
    }
    clear_effect_slots();
    clear_entity_effect_pending_records();
    clear_scene_record_pending_flags();
    finalize_pending_entity_template_update();
    if (DAT_00ffe186 != '\x03') {
      FUN_0005d260();
    }
    if (DAT_00ffe171 == '\0') {
      copy_terrain_layers();
    }
    else {
      DAT_00ffe171 = '\0';
      _DAT_00ffdf8a = 0;
    }
    if (DAT_00ffdf40 == 2) {
      process_terrain_work_entries();
    }
  }
  rebuild_terrain_collision_map();
  if ('\x01' < DAT_00ffe186) {
    load_scene_resources();
  }
  clear_scene_record_transition_words();
  populate_entity_table();
  dispatch_entity_action_candidates();
  if ((DAT_00ffde22 & 1) != 0) {
    decode_source_byte_stream();
    DAT_00ffddde = &DAT_00ff6700;
    DAT_00ffdde6 = &DAT_00ff6700;
  }
  mark_entity_slots_for_render();
  return;
}


