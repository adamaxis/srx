// Ghidra headless export
// Entry: 0x00023750
// Name: FUN_00023750
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


char FUN_00023750(void)

{
  char cVar1;
  short sVar2;
  undefined2 *puVar3;
  
  DAT_00fff0d6 = 0;
  cVar1 = '\0';
  puVar3 = &DAT_00ff0100;
  sVar2 = 2;
  do {
    if ((*(byte *)(puVar3 + 8) & 1) != 0) {
      cVar1 = cVar1 + '\x01';
    }
    puVar3 = puVar3 + 0x80;
    sVar2 = sVar2 + -1;
  } while (sVar2 != -1);
  return cVar1;
}


