// Ghidra headless export
// Entry: 0x00002FF0
// Name: FUN_00002ff0
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void FUN_00002ff0(void)

{
  short sVar1;
  undefined *puVar2;
  
  puVar2 = &DAT_00ff0400;
  sVar1 = 0xc;
  while (((puVar2[0x10] & 1) == 0 || ((puVar2[0xb0] & 0x80) == 0))) {
    puVar2 = puVar2 + 0x100;
    sVar1 = sVar1 + -1;
    if (sVar1 == -1) {
      stop();
      return;
    }
  }
  return;
}


