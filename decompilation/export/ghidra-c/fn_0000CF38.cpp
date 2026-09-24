// Ghidra headless export
// Entry: 0x0000CF38
// Name: FUN_0000cf38
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000BD2 wait_for_vblank
//   0x00000BF2 latch_controller_events
//   0x0000195C sample_controller_input
//   0x0000391A handle_mode_action_event
//   0x0000C0DA render_title_text_source
//   0x0000C376 move_wrapped_menu_cursor
//   0x0000C3E6 FUN_0000c3e6
//   0x0000C436 FUN_0000c436
//   0x0000CFFE FUN_0000cffe
//   0x0000E660 FUN_0000e660
//   0x0000EF08 FUN_0000ef08
//   0x00010952 FUN_00010952


void FUN_0000cf38(void)

{
  DAT_00fff10d = 0;
  FUN_00010952();
  FUN_0000cffe();
  FUN_0000ef08();
  do {
    FUN_0000cffe();
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
      if ((DAT_00ffdd22._0_1_ & 0x20) != 0) {
        handle_mode_action_event();
      }
    }
    FUN_0000c436();
  } while (-1 < DAT_00fff0dc);
  return;
}


