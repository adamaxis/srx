// Ghidra headless export
// Entry: 0x0005B00A
// Name: advance_title_intro_scroll
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0005B32C FUN_0005b32c


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void advance_title_intro_scroll(void)

{
  if (-0x201 < DAT_00ffdd08) {
    if ((DAT_00ffdd08 < -399) && (_DAT_00fff18a == 0)) {
      DAT_00ffdd08 = FUN_0005b32c();
      _DAT_00fff18a = 1;
    }
    DAT_00ffdd08 = DAT_00ffdd08 + -2;
    DAT_00ffef14 = DAT_00ffef14 | 4;
  }
  return;
}


