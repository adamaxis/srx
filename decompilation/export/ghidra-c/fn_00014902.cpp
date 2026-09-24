// Ghidra headless export
// Entry: 0x00014902
// Name: compute_obstruction_grid_index
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


short compute_obstruction_grid_index(void)

{
  ushort in_D0w;
  ushort in_D1w;
  
  return (in_D1w >> 4 & 0x7fe0) * 2 + (in_D0w >> 9);
}


