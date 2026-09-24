// Ghidra headless export
// Entry: 0x000042B0
// Name: FUN_000042b0
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_000042b0(void)

{
  uint in_D0;
  short sVar1;
  undefined4 *puVar2;
  undefined4 *puVar3;
  
  sVar1 = 0x3f;
  puVar2 = (undefined4 *)(&DAT_0007291a + (in_D0 & 0xffff) * 0x100);
  puVar3 = (undefined4 *)&DAT_00ff0100;
  do {
    *puVar3 = *puVar2;
    sVar1 = sVar1 + -1;
    puVar2 = puVar2 + 1;
    puVar3 = puVar3 + 1;
  } while (sVar1 != -1);
  DAT_00ff013e = 0;
  DAT_00ff023e = 1;
  _DAT_00ff033e = 2;
  return;
}


