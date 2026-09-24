// Ghidra headless export
// Entry: 0x000164A8
// Name: FUN_000164a8
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x000164F2 FUN_000164f2


void FUN_000164a8(void)

{
  int iVar1;
  undefined2 *extraout_A0;
  int *piVar2;
  bool bVar4;
  int *piVar3;
  
  piVar2 = DAT_00ffde02;
  while( true ) {
    piVar3 = piVar2 + 1;
    iVar1 = *piVar2;
    bVar4 = false;
    if (iVar1 == 0) {
      return;
    }
    FUN_000164f2();
    if (bVar4) break;
    *extraout_A0 = 0xffff;
    *(int *)(extraout_A0 + 2) = iVar1;
    piVar2 = piVar2 + 2;
    *(int *)(extraout_A0 + 8) = *piVar3;
    *(undefined1 **)(extraout_A0 + 4) = &LAB_0001641a;
    extraout_A0[7] = 0;
    extraout_A0[1] = 0;
  }
  return;
}


