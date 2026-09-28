// Ghidra headless export
// Entry: 0x00010016
// Name: run_magic_force_selection_loop
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000BD2 wait_for_vblank
//   0x00000BF2 latch_controller_events
//   0x0000195C sample_controller_input
//   0x0000FDFE advance_menu_sprite_overlay
//   0x000100A8 adjust_magic_force
//   0x00010172 refresh_magic_force_display
//
// Source behavior: enter through the C85A8 force-display variant, then run
// overlay/vblank/latch/sample once per frame. Bit B calls 0x100A8 with +1;
// bit A calls it with -1. Bits 9, E, and C leave the loop for the C85AC
// display variant. Bit D is a loop-back edge, not a finish input.


void run_magic_force_selection_loop(void)

{
  refresh_magic_force_display();
  do {
    while( true ) {
      advance_menu_sprite_overlay();
      wait_for_vblank();
      latch_controller_events();
      sample_controller_input();
      if (((DAT_00ffdd22._0_1_ & 8) == 0) && ((DAT_00ffdd22._0_1_ & 4) == 0)) break;
      adjust_magic_force();
    }
  } while (((DAT_00ffdd22._0_1_ & 2) == 0) &&
          ((((DAT_00ffdd22._0_1_ & 0x40) == 0 && ((DAT_00ffdd22._0_1_ & 0x10) == 0)) &&
           ((DAT_00ffdd22._0_1_ & 0x20) == 0))));
  refresh_magic_force_display();
  return;
}


