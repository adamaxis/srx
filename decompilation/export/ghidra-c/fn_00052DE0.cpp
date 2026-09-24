// Ghidra headless export
// Entry: 0x00052DE0
// Name: FUN_00052de0
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00052DC2 FUN_00052dc2


void FUN_00052de0(void)

{
  short sVar1;
  undefined1 *puVar2;
  undefined1 *puVar3;
  undefined1 *puVar4;
  
  IO_Z80RES = 0x100;
  FUN_00052dc2();
  sVar1 = 0x1885;
  puVar3 = &DAT_00026c5e;
  puVar2 = &DAT_00a00000;
  do {
    *puVar2 = *puVar3;
    sVar1 = sVar1 + -1;
    puVar3 = puVar3 + 1;
    puVar4 = puVar2 + 1;
    puVar2 = puVar2 + 1;
  } while (sVar1 != -1);
  do {
    puVar3 = puVar4 + 1;
    *puVar4 = 0;
    puVar4 = puVar3;
  } while (puVar3 != (undefined1 *)0xa02000);
  return;
}


