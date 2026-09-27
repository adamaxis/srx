// Ghidra headless export
// Entry: 0x0000CFFE
// Name: prepare_hotel_options_display
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000C0C2 load_title_font
//   0x00010A82 refresh_menu_grid_quadruple
//   0x000566D8 copy_mode_resource_metadata
//   0x0005670E refresh_mode_palette_data


void prepare_hotel_options_display(void)

{
  copy_mode_resource_metadata();
  refresh_mode_palette_data();
  refresh_menu_grid_quadruple();
  load_title_font();
  return;
}


