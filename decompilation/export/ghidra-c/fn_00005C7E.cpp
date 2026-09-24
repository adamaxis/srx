// Ghidra headless export
// Entry: 0x00005C7E
// Name: FUN_00005c7e
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001378 build_title_sprite_records


void FUN_00005c7e(void)

{
  short sVar1;
  int in_A0;
  
  if (*(short *)(in_A0 + 0xa6) < 0) {
    sVar1 = *(short *)(in_A0 + 0xa6) + -1;
    if (sVar1 < 0) {
      *(short *)(in_A0 + 0xa6) = sVar1;
      build_title_sprite_records();
      build_title_sprite_records();
      build_title_sprite_records();
      build_title_sprite_records();
      return;
    }
    *(undefined2 *)(in_A0 + 0xa6) = 0;
  }
  build_title_sprite_records();
  return;
}


