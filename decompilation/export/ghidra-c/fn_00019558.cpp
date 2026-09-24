// Ghidra headless export
// Entry: 0x00019558
// Name: FUN_00019558
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0001952C FUN_0001952c
//   0x0001954A FUN_0001954a


void FUN_00019558(void)

{
  short sVar1;
  
  sVar1 = 4;
  do {
    FUN_0001954a();
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  sVar1 = 0xb;
  do {
    FUN_0001952c();
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  return;
}


