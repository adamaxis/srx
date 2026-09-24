// Ghidra headless export
// Entry: 0x000530EA
// Name: FUN_000530ea
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00052F3C FUN_00052f3c
//   0x00053112 FUN_00053112


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined8 FUN_000530ea(void)

{
  undefined4 in_D0;
  undefined4 in_D1;
  undefined8 uVar1;
  
  if (_DAT_00fffc08 != 0) {
    uVar1 = FUN_00053112();
    FUN_00052f3c((int)((ulonglong)uVar1 >> 0x20));
    return uVar1;
  }
  return CONCAT44(in_D0,in_D1);
}


