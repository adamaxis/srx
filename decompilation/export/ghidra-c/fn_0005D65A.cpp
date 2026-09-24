// Ghidra headless export
// Entry: 0x0005D65A
// Name: FUN_0005d65a
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001378 build_title_sprite_records


void FUN_0005d65a(void)

{
  short sVar1;
  int in_A0;
  
  sVar1 = *(short *)(in_A0 + 0x20) + -1;
  *(short *)(in_A0 + 0x20) = sVar1;
  if (sVar1 < 0) {
    *(undefined2 *)(in_A0 + 0x20) = 4;
    *(short *)(in_A0 + 0x1e) = *(short *)(in_A0 + 0x1e) + 1;
    if (0x27 < *(short *)(in_A0 + 0x1e)) {
      *(undefined2 *)(in_A0 + 0x1e) = 0;
    }
  }
  build_title_sprite_records();
  return;
}


