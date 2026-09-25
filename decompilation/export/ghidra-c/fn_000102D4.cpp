// Ghidra headless export
// Entry: 0x000102D4
// Name: convert_menu_routine_0x000102d4
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000C0DA render_encoded_title_text
//   0x0000C216 build_grouped_decimal_glyphs
//   0x00010260 prepare_menu_grid_then_service
//   0x000107CE convert_menu_routine_0x000107ce
//   0x0001088C convert_menu_routine_0x0001088c


void convert_menu_routine_0x000102d4(void)

{
  prepare_menu_grid_then_service();
  render_encoded_title_text();
  render_encoded_title_text();
  render_encoded_title_text();
  convert_menu_routine_0x000107ce();
  convert_menu_routine_0x0001088c();
  build_grouped_decimal_glyphs();
  return;
}


