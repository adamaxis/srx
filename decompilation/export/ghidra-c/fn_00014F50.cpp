// Ghidra headless export
// Entry: 0x00014F50
// Name: FUN_00014f50
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00014F26 FUN_00014f26
//   0x00015ABA FUN_00015aba


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00014f50(void)

{
  char cVar1;
  short sVar2;
  undefined4 unaff_D5;
  undefined4 unaff_D6;
  undefined4 *extraout_A0;
  
  sVar2 = FUN_00014f26();
  if (((sVar2 == 0) || (_DAT_00ffdfe2 != 0)) &&
     (cVar1 = *(char *)((int)extraout_A0 + 0x89) + -1, *(char *)((int)extraout_A0 + 0x89) = cVar1,
     -1 < cVar1)) {
    *extraout_A0 = unaff_D5;
    extraout_A0[1] = unaff_D6;
    FUN_00015aba();
    return;
  }
  return;
}


