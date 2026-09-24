// Ghidra headless export
// Entry: 0x00019F7E
// Name: FUN_00019f7e
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00017BC0 refresh_menu_number_or_grid
//   0x000195AA FUN_000195aa
//   0x00019762 FUN_00019762


void FUN_00019f7e(void)

{
  short extraout_D1w;
  short sVar1;
  
  sVar1 = 4;
  do {
    if ((&DAT_00fffbd8)[sVar1] == '\a') {
      FUN_000195aa();
      (&DAT_00fff0e8)[sVar1] = 0;
      *(short *)((int)&DAT_00fff0f2 + (int)(short)(sVar1 << 1)) = extraout_D1w * 10;
      refresh_menu_number_or_grid();
      FUN_00019762();
      return;
    }
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  return;
}


