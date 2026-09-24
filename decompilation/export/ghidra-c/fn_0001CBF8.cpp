// Ghidra headless export
// Entry: 0x0001CBF8
// Name: FUN_0001cbf8
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0001cbf8(void)

{
  short sVar1;
  undefined4 *puVar2;
  
  if (DAT_00ff15b8 == '\0') {
    _DAT_00ffe010 = 0;
    puVar2 = (undefined4 *)&DAT_00ffe012;
    sVar1 = 6;
    do {
      *puVar2 = 0;
      puVar2 = puVar2 + 2;
      sVar1 = sVar1 + -1;
    } while (sVar1 != -1);
    return;
  }
  return;
}


