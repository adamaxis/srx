// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x000164A8


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_000164a8(void)

{
  int iVar1;
  undefined2 *extraout_A0;
  int *piVar2;
  bool bVar4;
  int *piVar3;
  
  piVar2 = _DAT_00ffde02;
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


