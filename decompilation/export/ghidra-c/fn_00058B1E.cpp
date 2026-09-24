// Ghidra headless export
// Entry: 0x00058B1E
// Name: FUN_00058b1e
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001232 FUN_00001232


void FUN_00058b1e(void)

{
  short sVar1;
  
  sVar1 = 5;
  do {
    FUN_00001232();
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  sVar1 = 1;
  do {
    FUN_00001232();
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  return;
}


