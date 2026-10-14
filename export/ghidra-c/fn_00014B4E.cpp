// Ghidra headless export
// Entry: 0x00014B4E
// Name: decode_obstruction_tile_projection
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void decode_obstruction_tile_projection(void)

{
  ushort in_D0w;
  ushort unaff_D5w;
  
  DAT_00ffdd2c = ((in_D0w >> 6) << 9 | in_D0w >> 0xd) + 0x100;
  DAT_00ffdd2a = (in_D0w & 0x3f) * 0x200 + 0x100;
  if (((unaff_D5w & 0xffc0) != (in_D0w & 0xffc0)) && ((in_D0w & 0xffc0) <= (unaff_D5w & 0xffc0))) {
    return;
  }
  return;
}


