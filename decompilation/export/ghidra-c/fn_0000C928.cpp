// Ghidra headless export
// Entry: 0x0000C928
// Name: FUN_0000c928
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000BD2 wait_for_vblank
//   0x00000BF2 latch_controller_events
//   0x0000195C sample_controller_input
//   0x0000391A FUN_0000391a
//   0x0000C0C2 FUN_0000c0c2
//   0x0000C0DA FUN_0000c0da
//   0x0000C376 move_wrapped_menu_cursor
//   0x0000C3E6 FUN_0000c3e6
//   0x0000C436 FUN_0000c436
//   0x0000E660 FUN_0000e660
//   0x00010952 FUN_00010952
//   0x00010A82 FUN_00010a82
//   0x000566D8 FUN_000566d8
//   0x0005670E FUN_0005670e


void FUN_0000c928(void)

{
  FUN_00010952();
  DAT_00fff0b7 = 1;
  do {
    FUN_000566d8();
    FUN_0005670e();
    FUN_00010a82();
    FUN_0000c0c2();
    FUN_0000c3e6();
    FUN_0000c0da();
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
        FUN_0000391a();
      }
    }
    FUN_0000c436();
    if (DAT_00fff0dc < '\0') {
      DAT_00fff0dc = 0;
      return;
    }
  } while( true );
}


