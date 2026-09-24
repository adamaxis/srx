// Ghidra headless export
// Entry: 0x0001882E
// Name: FUN_0001882e
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001232 write_vdp_grid
//   0x0000C0DA render_title_text_source
//   0x0000C134 normalize_source_text_line
//   0x0000C160 FUN_0000c160
//   0x0000C216 build_grouped_decimal_glyphs
//   0x0001897C write_hud_repeated_grid_variant_a
//   0x00018A92 FUN_00018a92
//   0x0001952C FUN_0001952c
//   0x0001957E clear_ready_menu_visual_slots
//   0x000195A6 refresh_menu_slot_pair
//   0x0001A16A FUN_0001a16a


void FUN_0001882e(void)

{
  short unaff_D7w;
  
  write_hud_repeated_grid_variant_a();
  FUN_00018a92();
  if ((unaff_D7w < 0xc) && ((char)(&DAT_00fffbcc)[unaff_D7w] < '\x01')) {
    render_title_text_source();
    return;
  }
  write_vdp_grid();
  write_vdp_grid();
  if ((short)(unaff_D7w + -0xc) < 0) {
    FUN_0001a16a();
    build_grouped_decimal_glyphs();
  }
  normalize_source_text_line();
  DAT_00ffef96 = 0xff;
  if ((short)(unaff_D7w + -0xc) < 0) {
    FUN_0000c160();
    normalize_source_text_line();
  }
  DAT_00ffef96 = 0xff;
  FUN_0000c160();
  if ((short)(unaff_D7w + -0xc) < 0) {
    refresh_menu_slot_pair();
    build_grouped_decimal_glyphs();
    FUN_0001952c();
    build_grouped_decimal_glyphs();
    clear_ready_menu_visual_slots();
    build_grouped_decimal_glyphs();
  }
  return;
}


