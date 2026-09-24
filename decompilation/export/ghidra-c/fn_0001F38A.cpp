// Ghidra headless export
// Entry: 0x0001F38A
// Name: FUN_0001f38a
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000F5C FUN_00000f5c


void FUN_0001f38a(void)

{
  short sVar1;
  
  sVar1 = 0x4f;
  do {
    FUN_00000f5c();
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  return;
}


