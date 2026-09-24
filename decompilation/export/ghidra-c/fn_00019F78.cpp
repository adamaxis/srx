// Ghidra headless export
// Entry: 0x00019F78
// Name: FUN_00019f78
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void FUN_00019f78(void)

{
  short extraout_D1w;
  short sVar1;
  
  sVar1 = 4;
  do {
    if ((&DAT_00fffbd8)[sVar1] == '\x06') {
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


