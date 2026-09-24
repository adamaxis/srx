// Ghidra headless export
// Entry: 0x0000C928
// Name: FUN_0000c928
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000BD2 wait_for_vblank
//   0x00000BF2 latch_controller_events
//   0x0000195C sample_controller_input
//   0x0000391A handle_mode_action_event
//   0x0000C0C2 load_title_font
//   0x0000C0DA render_title_text_source
//   0x0000C376 move_wrapped_menu_cursor
//   0x0000C3E6 FUN_0000c3e6
//   0x0000C436 FUN_0000c436
//   0x0000E660 FUN_0000e660
//   0x00010952 reset_mode_display_buffers
//   0x00010A82 FUN_00010a82
//   0x000566D8 FUN_000566d8
//   0x0005670E refresh_mode_palette_data


void FUN_0000c928(void)

{
  reset_mode_display_buffers();
  DAT_00fff0b7 = 1;
  do {
    FUN_000566d8();
    refresh_mode_palette_data();
    FUN_00010a82();
    load_title_font();
    FUN_0000c3e6();
    render_title_text_source();
    DAT_00fff0dc = '\0';
    move_wrapped_menu_cursor();
    while( true ) {
      wait_for_vblank();
      latch_controller_events();
      sample_controller_input();
      FUN_0000e660();
      if (((DAT_00ffdd22._0_1_ & 1) != 0) || ((DAT_00ffdd22._0_1_ & 2) != 0)) {
        move_wrapped_menu_cursor();
      }
      if ((DAT_00ffdd22._0_1_ & 0x40) != 0) break;
      if ((DAT_00ffdd22._0_1_ & 0x20) == 0) {
        if ((DAT_00ffdd22._0_1_ & 0x80) != 0) {
          DAT_00fff0dc = 0;
          return;
        }
      }
      else {
        handle_mode_action_event();
      }
    }
    FUN_0000c436();
    if (DAT_00fff0dc < '\0') {
      DAT_00fff0dc = 0;
      return;
    }
  } while( true );
}


