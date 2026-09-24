// Ghidra headless export
// Entry: 0x0001E41E
// Name: lookup_world_effect_tile
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


undefined1 lookup_world_effect_tile(void)

{
  ushort unaff_D5w;
  ushort unaff_D6w;
  
  return (&DAT_00ff1700)[(short)((unaff_D5w >> 9) + (unaff_D6w >> 4 & 0x7fe0) * 2)];
}


