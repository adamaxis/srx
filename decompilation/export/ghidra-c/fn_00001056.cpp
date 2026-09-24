// Ghidra headless export
// Entry: 0x00001056
// Name: FUN_00001056
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000E9C FUN_00000e9c


undefined2 FUN_00001056(void)

{
  short sVar1;
  
  FUN_00000e9c();
  sVar1 = 0xf;
  do {
    VDP_DATA = 0;
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  return 0;
}


