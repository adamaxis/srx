// Ghidra headless export
// Entry: 0x00000FFE
// Name: FUN_00000ffe
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000E9C FUN_00000e9c
//   0x00001056 FUN_00001056


undefined4 FUN_00000ffe(void)

{
  short sVar1;
  
  FUN_00001056();
  FUN_00000e9c();
  sVar1 = 0x3ff;
  do {
    Ram00c00000 = 0x7ff07ff;
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  return 0x7ff07ff;
}


