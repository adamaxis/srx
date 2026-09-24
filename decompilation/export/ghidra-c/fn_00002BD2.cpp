// Ghidra headless export
// Entry: 0x00002BD2
// Name: FUN_00002bd2
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000131C step_palette_transition
//   0x0000286E guard_world_effect_player_spawn
//   0x00002900 FUN_00002900
//   0x00002C6C FUN_00002c6c
//   0x0000301E upload_scene_profile_block


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00002bd2(void)

{
  short sVar1;
  
  step_palette_transition();
  if (_DAT_00ffdf8e == 0) {
    DAT_00ffddc6 = DAT_00ffddc6 & 0xff80;
    DAT_00ffddc8 = DAT_00ffddc8 & 0xff80;
    DAT_00ffdf36 = 0xffff;
    DAT_00ffdf38 = 0xffff;
    FUN_00002900();
    guard_world_effect_player_spawn();
    sVar1 = DAT_00ffe11a;
    if ((DAT_00ffe11a != 0) && (DAT_00ffe11a = 0, DAT_00ffe16e < '\0')) {
      (&DAT_00ff1700)[sVar1] = ((&DAT_00ff1700)[sVar1] & 0xe0) + 1;
    }
    FUN_00002c6c();
    upload_scene_profile_block();
    return;
  }
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


