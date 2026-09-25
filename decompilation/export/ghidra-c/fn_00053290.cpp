// Ghidra headless export
// Entry: 0x00053290
// Name: reset_audio_menu_arrays
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void reset_audio_menu_arrays(void)

{
  short sVar1;
  undefined4 *puVar2;
  undefined4 *puVar3;
  
  sVar1 = 0x22;
  puVar2 = &DAT_00fffaa4;
  puVar3 = &DAT_00fffb30;
  do {
    *puVar2 = 0;
    *puVar3 = 0;
    sVar1 = sVar1 + -1;
    puVar2 = puVar2 + 1;
    puVar3 = puVar3 + 1;
  } while (sVar1 != -1);
  return;
}


