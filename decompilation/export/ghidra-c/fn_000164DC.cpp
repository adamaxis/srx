// Ghidra headless export
// Entry: 0x000164DC
// Name: FUN_000164dc
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void FUN_000164dc(void)

{
  short sVar1;
  undefined2 *puVar2;
  
  puVar2 = &DAT_00ffd740;
  sVar1 = 9;
  do {
    *puVar2 = 0;
    puVar2 = puVar2 + 0xf;
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  return;
}


