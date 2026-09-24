// Ghidra headless export
// Entry: 0x00016A26
// Name: FUN_00016a26
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0001DBAA FUN_0001dbaa


void FUN_00016a26(void)

{
  int *piVar1;
  undefined *puVar2;
  int *piVar3;
  int *piVar4;
  
  DAT_00ffe18f = 1;
  piVar3 = &DAT_00ffe132;
  puVar2 = &DAT_00016a6a;
  for (piVar1 = DAT_00ffe162; *piVar1 != 0; piVar1 = piVar1 + 1) {
    piVar4 = piVar3 + 1;
    *piVar3 = *piVar1;
    piVar3 = piVar3 + 2;
    *piVar4 = (int)puVar2;
    puVar2 = puVar2 + 6;
  }
  *piVar3 = 0;
  DAT_00ffe120 = &DAT_00ffe132;
  FUN_0001dbaa();
  DAT_00ffe162 = (int *)0x0;
  return;
}


