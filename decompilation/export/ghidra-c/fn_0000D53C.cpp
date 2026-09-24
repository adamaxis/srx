// Ghidra headless export
// Entry: 0x0000D53C
// Name: FUN_0000d53c
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000BD2 wait_for_vblank
//   0x00000BF2 latch_controller_events
//   0x0000195C sample_controller_input
//   0x0000C0DA FUN_0000c0da
//   0x0000C376 move_wrapped_menu_cursor


void FUN_0000d53c(void)

{
  FUN_0000c0da();
  DAT_00fff0dc = 0;
  FUN_0000c0da();
  do {
    move_wrapped_menu_cursor();
    while( true ) {
      wait_for_vblank();
      latch_controller_events();
      sample_controller_input();
      if (((DAT_00ffdd22._0_1_ & 1) != 0) || ((DAT_00ffdd22._0_1_ & 2) != 0)) break;
      if ((DAT_00ffdd22._0_1_ & 0x40) != 0) {
        return;
      }
      if ((DAT_00ffdd22._0_1_ & 0x10) != 0) {
        return;
      }
      if ((DAT_00ffdd22._0_1_ & 0x20) != 0) {
        return;
      }
    }
  } while( true );
}


