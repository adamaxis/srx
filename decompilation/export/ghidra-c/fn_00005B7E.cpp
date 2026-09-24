// Ghidra headless export
// Entry: 0x00005B7E
// Name: FUN_00005b7e
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void FUN_00005b7e(void)

{
  short sVar1;
  undefined2 *puVar2;
  
  puVar2 = &DAT_00ff0100;
  sVar1 = 0xf;
  do {
    *(byte *)((int)puVar2 + 0x11) = *(byte *)((int)puVar2 + 0x11) | 1;
    puVar2 = puVar2 + 0x80;
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  return;
}


