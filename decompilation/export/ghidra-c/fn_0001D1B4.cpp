// Ghidra headless export
// Entry: 0x0001D1B4
// Name: retreat_effect_vertical_position
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void retreat_effect_vertical_position(void)

{
  int in_A0;
  
  if (0xaf < *(ushort *)(in_A0 + 2)) {
    *(short *)(in_A0 + 2) = *(short *)(in_A0 + 2) + -2;
  }
  return;
}


