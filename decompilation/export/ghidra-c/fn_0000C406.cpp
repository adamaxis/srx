// Ghidra headless export
// Entry: 0x0000C406
// Name: prepare_numeric_menu_block
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001232 write_vdp_grid
//   0x0000C216 build_grouped_decimal_glyphs


void prepare_numeric_menu_block(void)

{
  write_vdp_grid();
  build_grouped_decimal_glyphs();
  return;
}


