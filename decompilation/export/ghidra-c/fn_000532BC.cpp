// Ghidra headless export
// Entry: 0x000532BC
// Name: initialize_runner_selection_screen
// Decompile status: source-shaped annotation
//
// Native conversion: ghidra_conversion::initialize_runner_selection_screen.
// The original screen setup is retained as ordered host presentation work:
// clear/copy/upload the background asset, render the header, update the
// selected-runner animation, build sprite records, and issue the 10-by-14
// grid requests. Genesis VDP control words and source pointers are not part
// of the native path.
//
// Calls recovered from direct call instructions:
//   0x00000F5C write_vdp_word_block_with_source_wrap
//   0x00000FFE clear_and_fill_plane_a
//   0x00001232 write_vdp_grid
//   0x00001358 copy_source_word_block
//   0x0000C0DA render_encoded_title_text
//   0x00053200 select_primary_runner_record_table
//   0x0005322C prepare_audio_title
//   0x000533EE convert_audio_routine_0x000533ee

void initialize_runner_selection_screen(void)
{
  clear_runner_selection_background();
  copy_runner_selection_background_asset();
  upload_runner_selection_background_asset();
  render_runner_selection_initial_grid();
  render_runner_selection_header_text();
  update_runner_selection_animation();
  build_runner_selection_sprite_records_once();
  service_runner_selection_boundary();
  for (int row = 0; row < 10; ++row) {
    for (int column = 0; column < 14; ++column) {
      render_runner_selection_grid_cell(row, column);
    }
  }
}
