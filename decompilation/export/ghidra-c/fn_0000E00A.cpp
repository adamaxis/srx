// Ghidra headless export
// Entry: 0x0000E00A
// Name: FUN_0000e00a
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000BD2 wait_for_vblank
//   0x00000BF2 latch_controller_events
//   0x0000195C sample_controller_input
//   0x0000E6F4 FUN_0000e6f4


void FUN_0000e00a(void)

{
  do {
    wait_for_vblank();
    latch_controller_events();
    sample_controller_input();
    FUN_0000e6f4();
  } while ((DAT_00ffdd22 & 0xff00) == 0);
  sample_controller_input();
  return;
}


