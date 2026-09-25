// Ghidra headless export
// Entry: 0x000028C2
// Name: reinitialize_field_scene_entry
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000286E guard_world_effect_player_spawn
//   0x000028AA prepare_field_resources
//   0x00002900 convert_field_routine_0x00002900
//   0x00002C6C prepare_field_viewports
//   0x0000301E upload_scene_profile_block


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void reinitialize_field_scene_entry(void)

{
  _DAT_00ffdf8a = 0xffff;
  convert_field_routine_0x00002900();
  guard_world_effect_player_spawn();
  prepare_field_resources();
  DAT_00ffddc6 = DAT_00ffddc6 & 0xff80;
  DAT_00ffddc8 = DAT_00ffddc8 & 0xff80;
  DAT_00ffdf36 = 0xffff;
  DAT_00ffdf38 = 0xffff;
  prepare_field_viewports();
  upload_scene_profile_block();
  return;
}


