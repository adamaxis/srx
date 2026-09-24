// Ghidra headless export
// Entry: 0x00017C98
// Name: find_hud_selection_index
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void find_hud_selection_index(void)

{
  char in_D0b;
  short sVar1;
  short sVar2;
  
  sVar1 = 0;
  sVar2 = 4;
  do {
    if (in_D0b == (&DAT_00fffbd8)[sVar1]) {
      return;
    }
    sVar1 = sVar1 + 1;
    sVar2 = sVar2 + -1;
  } while (sVar2 != -1);
  return;
}


