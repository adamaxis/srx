// Ghidra headless export
// Entry: 0x0000286E
// Name: FUN_0000286e
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0001E4AC FUN_0001e4ac


void FUN_0000286e(void)

{
  short sVar1;
  undefined *puVar2;
  
  if (DAT_00fffc3c == 0) {
    return;
  }
  puVar2 = &DAT_00ff0400;
  sVar1 = 0xc;
  while (((puVar2[0x10] & 1) == 0 || ((puVar2[0xb0] & 0x80) == 0))) {
    puVar2 = puVar2 + 0x100;
    sVar1 = sVar1 + -1;
    if (sVar1 == -1) {
      FUN_0001e4ac();
      return;
    }
  }
  return;
}


