// Ghidra headless export
// Entry: 0x00018776
// Name: advance_hud_fade_service
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000BD2 wait_for_vblank
//   0x00000BF2 latch_controller_events
//   0x00001088 step_palette_toward_black


void advance_hud_fade_service(void)

{
  short sVar1;
  short sVar2;
  
  sVar2 = 4;
  do {
    sVar1 = 1;
    do {
      wait_for_vblank();
      latch_controller_events();
      sVar1 = sVar1 + -1;
    } while (sVar1 != -1);
    step_palette_toward_black();
    DAT_00ffef14 = DAT_00ffef14 | 1;
    sVar2 = sVar2 + -1;
  } while (sVar2 != -1);
  return;
}


