// Ghidra headless export
// Entry: 0x00059092
// Name: run_start_options
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000BD2 wait_for_vblank
//   0x00000BF2 latch_controller_events
//   0x0000195C sample_controller_input
//   0x000042B0 initialize_joshua_archetype_selection
//   0x0000C0C2 load_title_font
//   0x0000C0DA render_encoded_title_text
//   0x0000C376 move_wrapped_menu_cursor
//   0x0000C3E6 write_menu_grid_with_metadata
//   0x00010B5A transfer_menu_asset_batch_four
//   0x0001F34A prepare_mode_resource_variant_b
//   0x0001F38A prepare_mode_resource_variant_a
//   0x000566D8 convert_intro_routine_0x000566d8
//   0x000566E2 refresh_mode_resource_metadata
//   0x00059172 run_save_load


void run_start_options(void)

{
  convert_intro_routine_0x000566d8();
  refresh_mode_resource_metadata();
  transfer_menu_asset_batch_four();
  load_title_font();
  while( true ) {
    write_menu_grid_with_metadata();
    render_encoded_title_text();
    DAT_00fff0dc = '\0';
    move_wrapped_menu_cursor();
    do {
      wait_for_vblank();
      latch_controller_events();
      sample_controller_input();
      if (((DAT_00ffdd22._0_1_ & 1) != 0) || ((DAT_00ffdd22._0_1_ & 2) != 0)) {
        move_wrapped_menu_cursor();
      }
    } while ((((DAT_00ffdd22._0_1_ & 0x40) == 0) && ((DAT_00ffdd22._0_1_ & 0x10) == 0)) &&
            ((DAT_00ffdd22._0_1_ & 0x20) == 0));
    if (DAT_00fff0dc == '\0') break;
    run_save_load();
    if (-1 < DAT_00fff0dc) {
      return;
    }
  }
  write_menu_grid_with_metadata();
  render_encoded_title_text();
  DAT_00fff0dc = 0;
  do {
    move_wrapped_menu_cursor();
    DAT_00fff0ae = DAT_00fff0dc;
    prepare_mode_resource_variant_a();
    prepare_mode_resource_variant_b();
    while( true ) {
      wait_for_vblank();
      latch_controller_events();
      sample_controller_input();
      if (((DAT_00ffdd22._0_1_ & 1) != 0) || ((DAT_00ffdd22._0_1_ & 2) != 0)) break;
      if (((DAT_00ffdd22._0_1_ & 0x40) != 0) ||
         (((DAT_00ffdd22._0_1_ & 0x10) != 0 || ((DAT_00ffdd22._0_1_ & 0x20) != 0)))) {
        initialize_joshua_archetype_selection();
        return;
      }
    }
  } while( true );
}


