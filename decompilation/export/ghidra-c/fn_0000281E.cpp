// Ghidra headless export
// Entry: 0x0000281E
// Name: initialize_field_scene_entry
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000286E guard_world_effect_player_spawn
//   0x000028AA prepare_field_resources
//   0x00002900 run_world_frame_services
//   0x00002B34 apply_scene_profile_selection
//   0x00002C6C prepare_field_viewports
//   0x0000301E upload_scene_profile_block
//   0x00003074 prepare_field_scene_buffers


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void initialize_field_scene_entry(void)

{
  prepare_field_resources();
  _DAT_00ffdf8e = 0;
  _DAT_00ffdf8a = 0xffff;
  _DAT_00ffe172 = 0xffff;
  prepare_field_scene_buffers();
  run_world_frame_services();
  apply_scene_profile_selection();
  prepare_field_viewports();
  upload_scene_profile_block();
  guard_world_effect_player_spawn();
  return;
}


