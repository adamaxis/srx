// Ghidra headless export
// Entry: 0x000532A8
// Name: fill_audio_menu_sentinel
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void fill_audio_menu_sentinel(void)

{
  short sVar1;
  undefined4 *puVar2;
  
  sVar1 = 0x22;
  puVar2 = &DAT_00fffaa4;
  do {
    *puVar2 = 0xffffffff;
    sVar1 = sVar1 + -1;
    puVar2 = puVar2 + 1;
  } while (sVar1 != -1);
  return;
}


