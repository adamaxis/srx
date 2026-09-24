// Ghidra headless export
// Entry: 0x0001DEA0
// Name: FUN_0001dea0
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0001E514 FUN_0001e514


void FUN_0001dea0(void)

{
  short sVar1;
  undefined2 *puVar2;
  undefined2 *extraout_A0;
  
  puVar2 = &DAT_00ff0100;
  sVar1 = 9;
  do {
    if ((((*(byte *)(puVar2 + 8) & 0x40) != 0) && ((*(byte *)(puVar2 + 8) & 8) == 0)) &&
       (puVar2[6] == 0)) {
      FUN_0001e514();
      puVar2 = extraout_A0;
    }
    puVar2 = puVar2 + 0x80;
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  return;
}


