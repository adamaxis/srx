// Ghidra headless export
// Entry: 0x0005D5FC
// Name: advance_world_scene_effect_timer_b
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001378 build_title_sprite_records


void advance_world_scene_effect_timer_b(void)

{
  short sVar1;
  int in_A0;
  
  sVar1 = *(short *)(in_A0 + 0x1a) + -1;
  *(short *)(in_A0 + 0x1a) = sVar1;
  if (sVar1 < 0) {
    *(undefined2 *)(in_A0 + 0x1a) = 10;
    *(short *)(in_A0 + 0x1c) = *(short *)(in_A0 + 0x1c) + 1;
    if (9 < *(short *)(in_A0 + 0x1c)) {
      *(undefined2 *)(in_A0 + 0x1c) = 0;
    }
  }
  build_title_sprite_records();
  return;
}


