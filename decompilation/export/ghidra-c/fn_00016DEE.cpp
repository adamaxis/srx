// Ghidra headless export
// Entry: 0x00016DEE
// Name: FUN_00016dee
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void FUN_00016dee(void)

{
  short sVar1;
  undefined2 *puVar2;
  
  puVar2 = &DAT_00ff164c;
  sVar1 = 9;
  do {
    *(byte *)(puVar2 + 8) = *(byte *)(puVar2 + 8) & 0xfe;
    puVar2 = puVar2 + 9;
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  return;
}


