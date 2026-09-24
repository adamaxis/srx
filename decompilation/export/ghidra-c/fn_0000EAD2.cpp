// Ghidra headless export
// Entry: 0x0000EAD2
// Name: evaluate_terrain_direction_boundary
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000EDFA decode_terrain_tile_window


void evaluate_terrain_direction_boundary(void)

{
  short sVar1;
  ushort uVar2;
  short extraout_D1w;
  short unaff_D3w;
  short unaff_D5w;
  short unaff_D6w;
  
  sVar1 = DAT_00ffdfe4;
  uVar2 = decode_terrain_tile_window();
  uVar2 = uVar2 & 0x38;
  if (uVar2 == 0x20) {
    if ((ushort)-(extraout_D1w - unaff_D5w) < (ushort)(unaff_D3w - unaff_D6w)) {
      return;
    }
  }
  else if (uVar2 == 0x28) {
    if ((ushort)-(sVar1 - unaff_D6w) <= (ushort)-(extraout_D1w - unaff_D5w)) {
      return;
    }
  }
  else if (uVar2 == 0x38) {
    if ((ushort)(unaff_D3w - unaff_D6w) <= (ushort)-(extraout_D1w - unaff_D5w)) {
      return;
    }
  }
  else if ((ushort)-(extraout_D1w - unaff_D5w) < (ushort)-(sVar1 - unaff_D6w)) {
    return;
  }
  return;
}


