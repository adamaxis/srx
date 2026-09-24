// Ghidra headless export
// Entry: 0x0000F312
// Name: wait_for_controller_event
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000BD2 wait_for_vblank
//   0x00000BF2 latch_controller_events
//   0x0000195C sample_controller_input
//   0x0000E660 advance_random_grid_effect


void wait_for_controller_event(void)

{
  do {
    wait_for_vblank();
    latch_controller_events();
    sample_controller_input();
    advance_random_grid_effect();
  } while ((DAT_00ffdd22 & 0xff00) == 0);
  sample_controller_input();
  return;
}


