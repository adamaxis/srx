// Ghidra headless export
// Entry: 0x0001D1C4
// Name: advance_effect_horizontal_position
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void advance_effect_horizontal_position(void)

{
  ushort *in_A0;
  
  if (*in_A0 < 0x198) {
    *in_A0 = *in_A0 + 2;
  }
  return;
}


