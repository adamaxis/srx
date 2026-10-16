// Ghidra headless export
// Entry: 0x0001D1A4
// Name: advance_effect_vertical_position
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void advance_effect_vertical_position(void)

{
  int in_A0;
  
  if (*(ushort *)(in_A0 + 2) < 0xf4) {
    *(short *)(in_A0 + 2) = *(short *)(in_A0 + 2) + 2;
  }
  return;
}


