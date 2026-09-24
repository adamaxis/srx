// Ghidra headless export
// Entry: 0x00010722
// Name: FUN_00010722
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000BD2 wait_for_vblank
//   0x00000BF2 latch_controller_events
//   0x0000195C sample_controller_input
//   0x0000FDFE FUN_0000fdfe
//   0x000107CE FUN_000107ce


void FUN_00010722(void)

{
  FUN_000107ce();
  do {
    while( true ) {
      while( true ) {
        wait_for_vblank();
        latch_controller_events();
        sample_controller_input();
        FUN_0000fdfe();
        if ((DAT_00ffdd22._0_1_ & 1) != 0) {
          FUN_000107ce();
          return;
        }
        if ((DAT_00ffdd22._0_1_ & 8) == 0) break;
        FUN_000107ce();
      }
      if ((DAT_00ffdd22._0_1_ & 4) == 0) break;
      FUN_000107ce();
    }
  } while ((((DAT_00ffdd22._0_1_ & 0x40) == 0) && ((DAT_00ffdd22._0_1_ & 0x10) == 0)) &&
          ((DAT_00ffdd22._0_1_ & 0x20) == 0));
  FUN_000107ce();
  return;
}


