// Ghidra headless export
// Entry: 0x00012A54
// Name: FUN_00012a54
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0001107E FUN_0001107e


undefined4 FUN_00012a54(void)

{
  undefined4 in_D0;
  
  if ((short)((short)in_D0 + -0xd) < 0) {
    if (DAT_00fff100 == '\0') {
      return in_D0;
    }
    DAT_00fff100 = 0;
  }
  else {
    if (DAT_00fff100 != '\0') {
      return in_D0;
    }
    DAT_00fff100 = 1;
  }
  FUN_0001107e();
  return in_D0;
}


