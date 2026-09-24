// Ghidra headless export
// Entry: 0x00017708
// Name: FUN_00017708
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001232 FUN_00001232
//   0x00001728 FUN_00001728
//   0x00017D72 FUN_00017d72
//   0x00017D90 FUN_00017d90
//   0x00017F8E FUN_00017f8e
//   0x00019856 FUN_00019856
//   0x00019BAE FUN_00019bae


void FUN_00017708(void)

{
  short sVar1;
  
  VDP_CTRL = 0x8134;
  FUN_00001728();
  sVar1 = 4;
  do {
    FUN_00001232();
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  FUN_00019856();
  if ((DAT_00ffdf84 != '\0') && (sVar1 = FUN_00019bae(), sVar1 == 8)) {
    FUN_00017d72();
    FUN_00017d90();
    FUN_00017f8e();
  }
  return;
}


