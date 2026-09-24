// Ghidra headless export
// Entry: 0x00002C92
// Name: render_normal_field_viewport
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00003D18 render_field_tile_rows


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void render_normal_field_viewport(void)

{
  _DAT_00ffe100 = 0;
  render_field_tile_rows();
  return;
}


