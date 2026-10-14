// Ghidra headless export
// Entry: 0x000148D2
// Name: choose_dominant_axis_directions
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


undefined4 choose_dominant_axis_directions(void)

{
  ushort in_D0w;
  ushort uStack_6;
  
  return CONCAT22((short)in_D0w >> 0xf,(uStack_6 >> 4 & 0x7fe0) * 2 + (in_D0w >> 9));
}


