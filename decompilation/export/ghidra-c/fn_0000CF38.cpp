// Ghidra headless export
// Entry: 0x0000CF38
// Name: FUN_0000cf38
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000BD2 wait_for_vblank
//   0x00000BF2 latch_controller_events
//   0x0000195C sample_controller_input
//   0x0000391A handle_mode_action_event
//   0x0000C0DA render_encoded_title_text
//   0x0000C376 move_wrapped_menu_cursor
//   0x0000C3E6 write_menu_grid_with_metadata
//   0x0000C436 dispatch_menu_handler
//   0x0000CFFE prepare_title_mode_display
//   0x0000E660 advance_random_grid_effect
//   0x0000EF08 FUN_0000ef08
//   0x00010952 reset_mode_display_buffers


void FUN_0000cf38(void)

{
  DAT_00fff10d = 0;
  reset_mode_display_buffers();
  prepare_title_mode_display();
  FUN_0000ef08();
  do {
    prepare_title_mode_display();
    write_menu_grid_with_metadata();
    render_encoded_title_text();
    DAT_00fff0dc = '\0';
    move_wrapped_menu_cursor();
    while( true ) {
      wait_for_vblank();
      latch_controller_events();
      sample_controller_input();
      advance_random_grid_effect();
      if (((DAT_00ffdd22._0_1_ & 1) != 0) || ((DAT_00ffdd22._0_1_ & 2) != 0)) {
        move_wrapped_menu_cursor();
      }
      if ((DAT_00ffdd22._0_1_ & 0x40) != 0) break;
      if ((DAT_00ffdd22._0_1_ & 0x20) != 0) {
        handle_mode_action_event();
      }
    }
    dispatch_menu_handler();
  } while (-1 < DAT_00fff0dc);
  return;
}


