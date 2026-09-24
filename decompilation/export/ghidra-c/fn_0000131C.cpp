// Ghidra headless export
// Entry: 0x0000131C
// Name: step_palette_transition
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001088 FUN_00001088
//   0x00001342 FUN_00001342


void step_palette_transition(void)

{
  short sVar1;
  
  sVar1 = 0x10;
  do {
    FUN_00001342();
    FUN_00001088();
    DAT_00ffef14 = DAT_00ffef14 | 1;
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  return;
}


