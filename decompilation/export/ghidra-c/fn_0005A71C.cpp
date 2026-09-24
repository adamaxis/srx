// Ghidra headless export
// Entry: 0x0005A71C
// Name: advance_title_intro_linear_sweep_a
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001378 FUN_00001378


void advance_title_intro_linear_sweep_a(void)

{
  int iVar1;
  
  if (DAT_00ffdf88 < DAT_00fff16a) {
    if (DAT_00fff16c < 0x40) {
      return;
    }
  }
  else {
    if (7 < DAT_00fff168) {
      return;
    }
    iVar1 = (int)(short)(DAT_00fff168 << 3);
    DAT_00fff164 = *(undefined4 *)((int)&PTR_DAT_0005a7aa + iVar1);
    DAT_00fff16e = *(undefined2 *)(&DAT_0005a7ae + iVar1);
    DAT_00fff16a = DAT_00ffdf88 + *(short *)(&DAT_0005a7b0 + iVar1);
    DAT_00fff16c = 0x200;
    DAT_00fff168 = DAT_00fff168 + 1;
  }
  DAT_00fff16c = DAT_00fff16c - 8;
  FUN_00001378();
  return;
}


