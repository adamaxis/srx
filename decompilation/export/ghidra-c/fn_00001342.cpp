// Ghidra headless export
// Entry: 0x00001342
// Name: wait_for_palette_frame
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000BD2 wait_for_vblank
//   0x00000BF2 latch_controller_events


void wait_for_palette_frame(void)

{
  short unaff_D5w;
  
  do {
    wait_for_vblank();
    latch_controller_events();
    unaff_D5w = unaff_D5w + -1;
  } while (unaff_D5w != -1);
  return;
}


