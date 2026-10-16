// Ghidra headless export
// Entry: 0x0005D55C
// Name: advance_world_scene_effect_frame
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void advance_world_scene_effect_frame(void)

{
  short sVar1;
  int in_A0;
  
  sVar1 = *(short *)(in_A0 + 2) + -1;
  *(short *)(in_A0 + 2) = sVar1;
  if (sVar1 < 0) {
    *(undefined2 *)(in_A0 + 2) = 10;
    *(short *)(in_A0 + 0x18) = *(short *)(in_A0 + 0x18) + 1;
    if (7 < *(short *)(in_A0 + 0x18)) {
      *(undefined2 *)(in_A0 + 0x18) = 0;
    }
  }
  return;
}


