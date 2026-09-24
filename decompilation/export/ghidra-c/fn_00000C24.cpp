// Ghidra headless export
// Entry: 0x00000C24
// Name: wait_for_vblank_frames
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000BD2 wait_for_vblank


void wait_for_vblank_frames(void)

{
  short unaff_D5w;
  
  do {
    wait_for_vblank();
    unaff_D5w = unaff_D5w + -1;
  } while (unaff_D5w != -1);
  return;
}


