// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0005BA66


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0005ba66(void)

{
  int iVar1;
  short sVar2;
  undefined2 *puVar3;
  
  sVar2 = _DAT_00ffddea;
  _DAT_00ff2710 = DAT_001eb3e2;
  _DAT_00ff2712 = DAT_001eb3e4;
  iVar1 = (int)_DAT_00ffddea;
  *(undefined2 *)(&DAT_00ff2710 + iVar1) = DAT_001eb3e6;
  *(undefined2 *)(&DAT_00ff2712 + iVar1) = DAT_001eb3e8;
  puVar3 = (undefined2 *)((int)sVar2 + (int)(&DAT_00ff2710 + iVar1));
  *puVar3 = DAT_001eb3ea;
  puVar3[1] = PTR_DAT_001eb3ec._0_2_;
  _DAT_00ffe100 = 1;
  return;
}


