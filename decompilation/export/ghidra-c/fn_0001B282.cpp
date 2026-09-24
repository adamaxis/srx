// Ghidra headless export
// Entry: 0x0001B282
// Name: FUN_0001b282
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001378 FUN_00001378
//   0x0001792E FUN_0001792e
//   0x00017BC0 FUN_00017bc0
//   0x00019632 FUN_00019632
//   0x00019AEE FUN_00019aee
//   0x0001A908 FUN_0001a908


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0001b282(void)

{
  DAT_00ffe0da = DAT_00ffe0da + -1;
  if (DAT_00ffe0da < 0) {
    DAT_00ffe0da = 0xc;
    DAT_00ffe0dc = DAT_00ffe0dc + 1;
    if (DAT_00ffe0dc == 6) {
      FUN_00019632();
      FUN_00017bc0();
    }
    if (0xb < DAT_00ffe0dc) {
      FUN_00019aee();
      _DAT_00ffe0de = 0;
      FUN_0001792e();
      FUN_0001a908();
      return;
    }
  }
  FUN_00001378();
  return;
}


