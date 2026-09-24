// Ghidra headless export
// Entry: 0x00002CB2
// Name: render_alternate_field_viewport
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00003D18 render_field_tile_rows


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void render_alternate_field_viewport(void)

{
  _DAT_00ffe102 = 0;
  _DAT_00ffdf58 = 0x8000;
  render_field_tile_rows();
  _DAT_00ffdf58 = 0;
  return;
}


