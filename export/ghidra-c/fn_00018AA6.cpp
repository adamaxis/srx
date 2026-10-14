// Ghidra headless export
// Entry: 0x00018AA6
// Name: run_cyber_info_screen
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000BD2 wait_for_vblank
//   0x00000BF2 latch_controller_events
//   0x0000195C sample_controller_input
//   0x0000C0DA render_encoded_title_text
//   0x0000C216 build_grouped_decimal_glyphs
//   0x0000C376 move_wrapped_menu_cursor
//   0x0001882E render_matrix_panel
//   0x0001897C write_hud_repeated_grid_variant_a
//   0x000196F2 rotate_hud_history


void run_cyber_info_screen(void)

{
  short sVar1;
  
  DAT_00fff0dc = '\0';
  write_hud_repeated_grid_variant_a();
  render_encoded_title_text();
  do {
    move_wrapped_menu_cursor();
    while( true ) {
      wait_for_vblank();
      latch_controller_events();
      sample_controller_input();
      rotate_hud_history();
      if (((DAT_00ffdd22._0_1_ & 1) != 0) || ((DAT_00ffdd22._0_1_ & 2) != 0)) break;
      if (((DAT_00ffdd22._0_1_ & 0x40) != 0) ||
         (((DAT_00ffdd22._0_1_ & 0x10) != 0 || ((DAT_00ffdd22._0_1_ & 0x20) != 0)))) {
        if (DAT_00fff0dc == '\0') {
          write_hud_repeated_grid_variant_a();
          render_encoded_title_text();
          render_encoded_title_text();
          build_grouped_decimal_glyphs();
          build_grouped_decimal_glyphs();
          build_grouped_decimal_glyphs();
          build_grouped_decimal_glyphs();
          build_grouped_decimal_glyphs();
          build_grouped_decimal_glyphs();
          sVar1 = 3;
          do {
            build_grouped_decimal_glyphs();
            sVar1 = sVar1 + -1;
          } while (sVar1 != -1);
          return;
        }
        write_hud_repeated_grid_variant_a();
        render_encoded_title_text();
        if (DAT_00fff0e3 < '\0') {
          render_encoded_title_text();
          if (DAT_00fffbf2 < '\0') {
            render_encoded_title_text();
          }
          else {
            render_encoded_title_text();
          }
          render_encoded_title_text();
          render_encoded_title_text();
          return;
        }
        if (DAT_00fff0e3 < '\x01') {
          render_encoded_title_text();
          return;
        }
        render_encoded_title_text();
        return;
      }
      if ((DAT_00ffdd22._0_1_ & 0x80) != 0) {
        render_matrix_panel();
        return;
      }
    }
  } while( true );
}

