// Ghidra headless export
// Entry: 0x0000AC84
// Name: FUN_0000ac84
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000AE5E FUN_0000ae5e
//   0x000138B8 FUN_000138b8
//   0x00013904 FUN_00013904


void FUN_0000ac84(void)

{
  int in_A0;
  
  if ((*(byte *)(in_A0 + 0x10) & 8) == 0) {
    FUN_00013904();
  }
  else {
    FUN_000138b8();
  }
  if (DAT_00ffe18a != '\0') {
    FUN_0000ae5e();
  }
  return;
}


