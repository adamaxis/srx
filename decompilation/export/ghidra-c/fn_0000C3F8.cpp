// Ghidra headless export
// Entry: 0x0000C3F8
// Name: render_menu_grid_number_pair
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void render_menu_grid_number_pair(void)

{
  write_vdp_grid();
  build_grouped_decimal_glyphs();
  return;
}


