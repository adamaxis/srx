// Ghidra headless export
// Entry: 0x0001D2E6
// Name: advance_world_sequence
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0001D33A render_world_sequence_tile_rows


void advance_world_sequence(void)

{
  short *psVar1;
  
  for (psVar1 = DAT_00ffdf7a; *psVar1 != -0x8000; psVar1 = psVar1 + 4) {
    render_world_sequence_tile_rows();
  }
  DAT_00ffdf82 = DAT_00ffdf82 + -1;
  if (DAT_00ffdf82 < 0) {
    DAT_00ffdf82 = 0;
    DAT_00ffdf70 = DAT_00ffdf70 + 1;
  }
  if (DAT_00ffdf70 <= DAT_00ffdf78) {
    return;
  }
  return;
}

