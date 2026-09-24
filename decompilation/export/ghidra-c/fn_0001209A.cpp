// Ghidra headless export
// Entry: 0x0001209A
// Name: FUN_0001209a
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000C0DA render_title_text_source
//   0x000110E8 FUN_000110e8


void FUN_0001209a(void)

{
  short *psVar1;
  
  do {
    psVar1 = &DAT_00ffdd48;
    if (DAT_00ffdd28 == 0) {
      DAT_00ffdd28 = -1;
    }
    else {
      DAT_00ffdd28 = 0;
      psVar1 = (short *)&DAT_00ffdd4c;
    }
  } while (*psVar1 < 0);
  render_title_text_source();
  FUN_000110e8();
  return;
}


