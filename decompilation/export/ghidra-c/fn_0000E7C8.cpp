// Ghidra headless export
// Entry: 0x0000E7C8
// Name: evaluate_terrain_collision
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000E8A0 sample_terrain_collision_tiles
//   0x0000E9EE contains_terrain_object_bounds
//   0x0000EA36 evaluate_entity_terrain_collision_directions


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

ushort evaluate_terrain_collision(void)

{
  char cVar1;
  ushort unaff_D5w;
  ushort unaff_D6w;
  bool bVar2;
  
  _DAT_00ffdf68 = 0;
  _DAT_00ffdfe2 = 0;
  if ((((DAT_00ffddd2 <= unaff_D5w >> 4) || (unaff_D5w >> 4 < DAT_00ffddd6)) ||
      (DAT_00ffddd4 <= unaff_D6w >> 4)) || (unaff_D6w >> 4 < DAT_00ffddd8)) {
    _DAT_00ffef12 = 0x7f;
    _DAT_00ffef10 = 0x7f;
    _DAT_00ffdfe2 = 0;
    _DAT_00ffdf68 = 0xffff;
    return 0x7f;
  }
  cVar1 = sample_terrain_collision_tiles();
  DAT_00ffdf64 = (ushort)cVar1;
  if (DAT_00ffdf64 == 0) {
    return 0;
  }
  if (DAT_00ffdf64 != 0x7f) {
    bVar2 = false;
    if ((short)DAT_00ffdf64 < 0) {
      return DAT_00ffdf64;
    }
    if ((DAT_00ffdf64 & 0x40) == 0) {
      if ((DAT_00ffdf64 & 0x20) == 0) {
        contains_terrain_object_bounds();
        if (!bVar2) {
          return 0;
        }
        return 0x7f;
      }
    }
    else {
      _DAT_00ffdfe2 = 0xffff;
      bVar2 = DAT_00ffdf64 < 0x7e;
      if (DAT_00ffdf64 == 0x7e) {
        _DAT_00ffdfe2 = 0xffff;
        return 0x7f;
      }
      if ((DAT_00ffdf64 & 0x20) == 0) {
        _DAT_00ffdfe2 = 0xffff;
        return 0x7f;
      }
    }
    evaluate_entity_terrain_collision_directions();
    if (bVar2) {
      return 0x7f;
    }
    return 0;
  }
  return 0x7f;
}


