// Ghidra headless export
// Entry: 0x0005BA66
// Name: FUN_0005ba66
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0005ba66(void)

{
  int iVar1;
  short sVar2;
  undefined2 *puVar3;
  
  sVar2 = DAT_00ffddea;
  DAT_00ff2710 = DAT_001eb3e2;
  DAT_00ff2712 = DAT_001eb3e4;
  iVar1 = (int)DAT_00ffddea;
  puVar3 = (undefined2 *)((int)&DAT_00ff2710 + iVar1);
  *puVar3 = DAT_001eb3e6;
  *(undefined2 *)((int)&DAT_00ff2712 + iVar1) = DAT_001eb3e8;
  puVar3 = (undefined2 *)((int)sVar2 + (int)puVar3);
  *puVar3 = DAT_001eb3ea;
  puVar3[1] = PTR_DAT_001eb3ec._0_2_;
  _DAT_00ffe100 = 1;
  return;
}


