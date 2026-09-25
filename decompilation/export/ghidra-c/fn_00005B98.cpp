// Ghidra headless export
// Entry: 0x00005B98
// Name: emit_title_sprite_burst
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001378 build_title_sprite_records


void emit_title_sprite_burst(void)

{
  short sVar1;
  int in_A0;
  
  if ((*(short *)(in_A0 + 0x38) != 0x2a) && (-1 < (short)(*(short *)(in_A0 + 0x44) + -8))) {
    sVar1 = 0xf;
    do {
      build_title_sprite_records();
      sVar1 = sVar1 + -1;
    } while (sVar1 != -1);
    return;
  }
  return;
}


