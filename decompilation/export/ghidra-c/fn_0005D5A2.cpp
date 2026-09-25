// Ghidra headless export
// Entry: 0x0005D5A2
// Name: convert_intro_routine_0x0005d5a2
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001378 build_title_sprite_records


void convert_intro_routine_0x0005d5a2(void)

{
  short sVar1;
  int in_A0;
  
  sVar1 = *(short *)(in_A0 + 6) + -1;
  *(short *)(in_A0 + 6) = sVar1;
  if (sVar1 < 0) {
    *(undefined2 *)(in_A0 + 6) = 0x14;
    *(short *)(in_A0 + 0x16) = *(short *)(in_A0 + 0x16) + 1;
    if (8 < *(short *)(in_A0 + 0x16)) {
      *(undefined2 *)(in_A0 + 0x16) = 0;
    }
  }
  build_title_sprite_records();
  return;
}


