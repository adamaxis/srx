// Ghidra headless export
// Entry: 0x00014640
// Name: map_world_delta_to_tile_pair
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


undefined6 map_world_delta_to_tile_pair(void)

{
  ushort in_D0w;
  ushort uVar1;
  
  uVar1 = (in_D0w >> 1) / (DAT_00ffddea >> 1);
  return CONCAT24(uVar1 * 0x200 + 0x100,
                  CONCAT22(uVar1,((in_D0w >> 1) % (DAT_00ffddea >> 1)) * 0x200 + 0x100));
}


