// Ghidra headless export
// Entry: 0x0001D1D0
// Name: retreat_effect_horizontal_position
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void retreat_effect_horizontal_position(void)

{
  ushort *in_A0;
  
  if (0xa7 < *in_A0) {
    *in_A0 = *in_A0 - 2;
  }
  return;
}


