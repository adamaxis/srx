// Ghidra headless export
// Entry: 0x0001882E
// Name: render_matrix_panel
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001232 write_vdp_grid
//   0x0000C0DA render_encoded_title_text
//   0x0000C134 normalize_source_text_line
//   0x0000C160 render_wrapped_text
//   0x0000C216 build_grouped_decimal_glyphs
//   0x0001897C write_hud_repeated_grid_variant_a
//   0x00018A92 matrix_program_index
//   0x0001952C matrix_program_storage_usage
//   0x0001957E calculate_loaded_program_capacity
//   0x000195A6 calculate_selected_matrix_program_load_steps
//   0x0001A16A render_matrix_runner_detail_panel


void render_matrix_panel(void)

{
  short unaff_D7w;
  
  write_hud_repeated_grid_variant_a();
  matrix_program_index();
  if ((unaff_D7w < 0xc) && ((char)(&DAT_00fffbcc)[unaff_D7w] < '\x01')) {
    render_encoded_title_text();
    return;
  }
  write_vdp_grid();
  write_vdp_grid();
  if ((short)(unaff_D7w + -0xc) < 0) {
    render_matrix_runner_detail_panel();
    build_grouped_decimal_glyphs();
  }
  normalize_source_text_line();
  DAT_00ffef96 = 0xff;
  if ((short)(unaff_D7w + -0xc) < 0) {
    render_wrapped_text();
    normalize_source_text_line();
  }
  DAT_00ffef96 = 0xff;
  render_wrapped_text();
  if ((short)(unaff_D7w + -0xc) < 0) {
    calculate_selected_matrix_program_load_steps();
    build_grouped_decimal_glyphs();
    matrix_program_storage_usage();
    build_grouped_decimal_glyphs();
    calculate_loaded_program_capacity();
    build_grouped_decimal_glyphs();
  }
  return;
}

