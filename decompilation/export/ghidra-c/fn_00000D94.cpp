// Ghidra headless export
// Entry: 0x00000D94
// Name: FUN_00000d94
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000BD2 wait_for_vblank
//   0x00000BF2 latch_controller_events
//   0x0000195C sample_controller_input


void FUN_00000d94(void)

{
  do {
    wait_for_vblank();
    latch_controller_events();
    sample_controller_input();
  } while ((DAT_00ffdd22 & 0xff00) == 0);
  sample_controller_input();
  return;
}


