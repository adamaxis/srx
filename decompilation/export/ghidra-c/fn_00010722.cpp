// Ghidra headless export
// Entry: 0x00010722
// Name: convert_menu_routine_0x00010722
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000BD2 wait_for_vblank
//   0x00000BF2 latch_controller_events
//   0x0000195C sample_controller_input
//   0x0000FDFE convert_graphics_routine_0x0000fdfe
//   0x000107CE adjust_runner_posture


void convert_menu_routine_0x00010722(void)

{
  adjust_runner_posture();
  do {
    while( true ) {
      while( true ) {
        wait_for_vblank();
        latch_controller_events();
        sample_controller_input();
        convert_graphics_routine_0x0000fdfe();
        if ((DAT_00ffdd22._0_1_ & 1) != 0) {
          adjust_runner_posture();
          return;
        }
        if ((DAT_00ffdd22._0_1_ & 8) == 0) break;
        adjust_runner_posture();
      }
      if ((DAT_00ffdd22._0_1_ & 4) == 0) break;
      adjust_runner_posture();
    }
  } while ((((DAT_00ffdd22._0_1_ & 0x40) == 0) && ((DAT_00ffdd22._0_1_ & 0x10) == 0)) &&
          ((DAT_00ffdd22._0_1_ & 0x20) == 0));
  adjust_runner_posture();
  return;
}


