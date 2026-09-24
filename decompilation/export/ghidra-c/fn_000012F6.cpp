// Ghidra headless export
// Entry: 0x000012F6
// Name: FUN_000012f6
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x000010D6 step_palette_toward_target
//   0x00001342 wait_for_palette_frame


void FUN_000012f6(void)

{
  short sVar1;
  
  sVar1 = 0x10;
  do {
    wait_for_palette_frame();
    step_palette_toward_target();
    DAT_00ffef14 = DAT_00ffef14 | 1;
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  return;
}


