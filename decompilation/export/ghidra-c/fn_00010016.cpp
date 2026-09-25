// Ghidra headless export
// Entry: 0x00010016
// Name: convert_menu_routine_0x00010016
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000BD2 wait_for_vblank
//   0x00000BF2 latch_controller_events
//   0x0000195C sample_controller_input
//   0x0000FDFE convert_graphics_routine_0x0000fdfe
//   0x000100A8 convert_menu_routine_0x000100a8
//   0x00010172 convert_menu_routine_0x00010172


void convert_menu_routine_0x00010016(void)

{
  convert_menu_routine_0x00010172();
  do {
    while( true ) {
      convert_graphics_routine_0x0000fdfe();
      wait_for_vblank();
      latch_controller_events();
      sample_controller_input();
      if (((DAT_00ffdd22._0_1_ & 8) == 0) && ((DAT_00ffdd22._0_1_ & 4) == 0)) break;
      convert_menu_routine_0x000100a8();
    }
  } while (((DAT_00ffdd22._0_1_ & 2) == 0) &&
          ((((DAT_00ffdd22._0_1_ & 0x40) == 0 && ((DAT_00ffdd22._0_1_ & 0x10) == 0)) &&
           ((DAT_00ffdd22._0_1_ & 0x20) == 0))));
  convert_menu_routine_0x00010172();
  return;
}


