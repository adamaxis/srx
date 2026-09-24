// Ghidra headless export
// Entry: 0x0001DF00
// Name: FUN_0001df00
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void FUN_0001df00(void)

{
  short sVar1;
  undefined *puVar2;
  
  puVar2 = &DAT_00ff0400;
  sVar1 = 6;
  do {
    puVar2[0x9e] = puVar2[0x9e] & 0xfe;
    puVar2 = puVar2 + 0x100;
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  return;
}


