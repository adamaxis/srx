// Ghidra headless export
// Entry: 0x0000F312
// Name: FUN_0000f312
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000BD2 wait_for_vblank
//   0x00000BF2 latch_controller_events
//   0x0000195C sample_controller_input
//   0x0000E660 FUN_0000e660


void FUN_0000f312(void)

{
  do {
    wait_for_vblank();
    latch_controller_events();
    sample_controller_input();
    FUN_0000e660();
  } while ((DAT_00ffdd22 & 0xff00) == 0);
  sample_controller_input();
  return;
}


