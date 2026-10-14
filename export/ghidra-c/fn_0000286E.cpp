// Ghidra headless export
// Entry: 0x0000286E
// Name: guard_world_effect_player_spawn
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0001E4AC spawn_world_effect_at_player_position


void guard_world_effect_player_spawn(void)

{
  short sVar1;
  undefined *puVar2;
  
  if (DAT_00fffc3c == 0) {
    return;
  }
  puVar2 = &DAT_00ff0400;
  sVar1 = 0xc;
  while (((puVar2[0x10] & 1) == 0 || ((puVar2[0xb0] & 0x80) == 0))) {
    puVar2 = puVar2 + 0x100;
    sVar1 = sVar1 + -1;
    if (sVar1 == -1) {
      spawn_world_effect_at_player_position();
      return;
    }
  }
  return;
}


