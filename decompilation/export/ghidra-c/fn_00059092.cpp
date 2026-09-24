// Ghidra headless export
// Entry: 0x00059092
// Name: FUN_00059092
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000BD2 wait_for_vblank
//   0x00000BF2 latch_controller_events
//   0x0000195C sample_controller_input
//   0x000042B0 FUN_000042b0
//   0x0000C0C2 FUN_0000c0c2
//   0x0000C0DA FUN_0000c0da
//   0x0000C376 FUN_0000c376
//   0x0000C3E6 FUN_0000c3e6
//   0x00010B5A FUN_00010b5a
//   0x0001F34A FUN_0001f34a
//   0x0001F38A FUN_0001f38a
//   0x000566D8 FUN_000566d8
//   0x000566E2 FUN_000566e2
//   0x00059172 FUN_00059172


void FUN_00059092(void)

{
  FUN_000566d8();
  FUN_000566e2();
  FUN_00010b5a();
  FUN_0000c0c2();
  while( true ) {
    FUN_0000c3e6();
    FUN_0000c0da();
    DAT_00fff0dc = '\0';
    FUN_0000c376();
    do {
      wait_for_vblank();
      latch_controller_events();
      sample_controller_input();
      if (((DAT_00ffdd22._0_1_ & 1) != 0) || ((DAT_00ffdd22._0_1_ & 2) != 0)) {
        FUN_0000c376();
      }
    } while ((((DAT_00ffdd22._0_1_ & 0x40) == 0) && ((DAT_00ffdd22._0_1_ & 0x10) == 0)) &&
            ((DAT_00ffdd22._0_1_ & 0x20) == 0));
    if (DAT_00fff0dc == '\0') break;
    FUN_00059172();
    if (-1 < DAT_00fff0dc) {
      return;
    }
  }
  FUN_0000c3e6();
  FUN_0000c0da();
  DAT_00fff0dc = 0;
  do {
    FUN_0000c376();
    DAT_00fff0ae = DAT_00fff0dc;
    FUN_0001f38a();
    FUN_0001f34a();
    while( true ) {
      wait_for_vblank();
      latch_controller_events();
      sample_controller_input();
      if (((DAT_00ffdd22._0_1_ & 1) != 0) || ((DAT_00ffdd22._0_1_ & 2) != 0)) break;
      if (((DAT_00ffdd22._0_1_ & 0x40) != 0) ||
         (((DAT_00ffdd22._0_1_ & 0x10) != 0 || ((DAT_00ffdd22._0_1_ & 0x20) != 0)))) {
        FUN_000042b0();
        return;
      }
    }
  } while( true );
}


