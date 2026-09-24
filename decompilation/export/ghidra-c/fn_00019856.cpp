// Ghidra headless export
// Entry: 0x00019856
// Name: FUN_00019856
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001232 FUN_00001232
//   0x00017DB8 FUN_00017db8


void FUN_00019856(void)

{
  short sVar1;
  
  FUN_00017db8();
  sVar1 = 4;
  do {
    FUN_00001232();
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  return;
}


