// Ghidra headless export
// Entry: 0x00016E30
// Name: FUN_00016e30
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0001A310 FUN_0001a310
//   0x0001A322 FUN_0001a322


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00016e30(void)

{
  short sVar1;
  undefined2 *puVar2;
  undefined2 *puVar3;
  
  _DAT_00ffdf8a = 0xffff;
  sVar1 = 0x30;
  puVar2 = &DAT_00ffddc4;
  puVar3 = &DAT_00ffde26;
  do {
    *puVar3 = *puVar2;
    sVar1 = sVar1 + -1;
    puVar2 = puVar2 + 1;
    puVar3 = puVar3 + 1;
  } while (sVar1 != -1);
  FUN_0001a322();
  FUN_0001a310();
  return;
}


