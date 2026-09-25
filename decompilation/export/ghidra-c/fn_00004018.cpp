// Ghidra headless export
// Entry: 0x00004018
// Name: render_field_tile_edge
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00003D4A render_field_tile_column
//   0x00004000 convert_field_routine_0x00004000
//   0x00004216 offset_field_coordinate


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void render_field_tile_edge(void)

{
  short in_D0w;
  
  if (in_D0w < 0) {
    if (-1 < (short)((DAT_00ffddc8 >> 4) - 8)) {
      offset_field_coordinate();
      convert_field_routine_0x00004000();
      render_field_tile_column();
      _DAT_00ffdf58 = 0x8000;
      render_field_tile_column();
      _DAT_00ffdf58 = 0;
    }
  }
  else if ((short)((DAT_00ffddc8 >> 4) + 8) <= DAT_00ffddd0) {
    offset_field_coordinate();
    convert_field_routine_0x00004000();
    render_field_tile_column();
    _DAT_00ffdf58 = 0x8000;
    render_field_tile_column();
    _DAT_00ffdf58 = 0;
  }
  return;
}


