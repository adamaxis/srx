// Ghidra headless export
// Entry: 0x0000281E
// Name: FUN_0000281e
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000286E guard_world_effect_player_spawn
//   0x000028AA FUN_000028aa
//   0x00002900 FUN_00002900
//   0x00002B34 apply_scene_profile_selection
//   0x00002C6C FUN_00002c6c
//   0x0000301E upload_scene_profile_block
//   0x00003074 FUN_00003074


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0000281e(void)

{
  FUN_000028aa();
  _DAT_00ffdf8e = 0;
  _DAT_00ffdf8a = 0xffff;
  _DAT_00ffe172 = 0xffff;
  FUN_00003074();
  FUN_00002900();
  apply_scene_profile_selection();
  FUN_00002c6c();
  upload_scene_profile_block();
  guard_world_effect_player_spawn();
  return;
}


